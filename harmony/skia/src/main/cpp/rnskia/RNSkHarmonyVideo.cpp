/*
 * Copyright (c) 2024 Huawei Device Co., Ltd. All rights reserved
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

#include "RNSkHarmonyVideo.h"
#include "RNSkOpenGLCanvasProvider.h"
#include <glog/logging.h>
#include <chrono>
#include <bits/alltypes.h>
#include <multimedia/player_framework/native_averrors.h>
#include "plugin_manager.h"
#include <cstdint>
#include <fcntl.h>
#include <regex>
#include <vector>
#include "sample_callback.h"
#undef LOG_TAG
#define LOG_TAG "player"

namespace {
using namespace std::chrono_literals;
}
namespace RNSkia {

RNSkHarmonyVideo RNSkHarmonyVideo::HarmonyVideo;
RNSkHarmonyVideo::~RNSkHarmonyVideo() {
    DLOG(INFO) << "~RNSkHarmonyVideo";
    StartRelease();
    DLOG(INFO) << "~RNSkHarmonyVideo end";
}

int32_t RNSkHarmonyVideo::OpenFile(SampleInfo &sampleInfo) {
    DLOG(INFO) << "OpenFile enter.";
    if (sampleInfo.uri.empty()) {
        DLOG(INFO) << "OpenFile: url empty";
        return AV_ERR_UNKNOWN;
    }

    std::string assetsFilePath = sampleInfo.uri;
    std::string Prefixes = "../../";

    if (sampleInfo.uri.find(Prefixes) == 0) {
        assetsFilePath = DEFAULT_ASSETS_DEST + sampleInfo.uri.substr(Prefixes.length());
        DLOG(INFO) << "ReadAssetsData assetsFilePath=" << assetsFilePath;
    }

    if (nativeResMgr == nullptr) {
        DLOG(ERROR) << "ReadAssetsData env error, nativeResMgr: " << nativeResMgr;
    }

    RawFile *_file = OH_ResourceManager_OpenRawFile(nativeResMgr, assetsFilePath.c_str());
    if (_file == nullptr) {
        DLOG(ERROR) << "ReadAssetsData open file error.";
    }
    RawFileDescriptor descriptor;
    OH_ResourceManager_GetRawFileDescriptor(_file, descriptor);

    sampleInfo.inputFd = descriptor.fd;
    sampleInfo.inputFileSize = descriptor.length;
    sampleInfo.inputFileOffset = descriptor.start;
    DLOG(INFO) << "Open file: fd = " << sampleInfo.inputFd << " sampleInfo.inputFileSize: " << sampleInfo.inputFileSize;
    if (sampleInfo.inputFd == -1) {
        DLOG(ERROR) << "cont open file, error: " << strerror(errno);
        return AV_ERR_UNKNOWN;
    }

    DLOG(INFO) << "OpenFile end.";
    return AV_ERR_OK;
}

int32_t RNSkHarmonyVideo::Init(SampleInfo &sampleInfo) {
    DLOG(INFO) << "Init enter.";
    std::lock_guard<std::mutex> lock(mutex_);
    if (isStarted_) {
        DLOG(ERROR) << "Already started.";
        return AV_ERR_UNKNOWN;
    }
    std::string HTTPFilePath = sampleInfo.uri;
    std::string https = "https";
    if (sampleInfo.uri.find(https) == 0) {
        HTTPFilePath = DEFAULT_HTTP_DEST + sampleInfo.uri.substr(https.length());

        DLOG(INFO) << "OpenFile HTTPFilePath=" << https;
        sampleInfo.uri = HTTPFilePath;
    } else {
        OpenFile(sampleInfo);
    }

    sampleInfo_ = sampleInfo;

    videoDecoder_ = std::make_unique<VideoDecoder>();
    if (!videoDecoder_) {
        DLOG(ERROR) << "Create videoDecoder_ failed";
        return AV_ERR_UNKNOWN;
    }
    demuxer_ = std::make_unique<Demuxer>();
    int32_t ret = demuxer_->CreateDemuxer(sampleInfo_);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) << "Create demuxer failed";
        return AV_ERR_UNKNOWN;
    }
    ret = videoDecoder_->CreateVideoDecoder(sampleInfo_.codecMime);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) << "Create video decoder failed";
        return AV_ERR_UNKNOWN;
    }
    if (demuxer_->hasAudio()) {
        ret = InitAudio();
        if (ret != AV_ERR_OK) {
            return ret;
        }
    }
    videoSignal_ = new VDecSignal;
    ret = videoDecoder_->Config(sampleInfo_, videoSignal_);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) << "Decoder config failed";
        return AV_ERR_UNKNOWN;
    }
    InitControlSignal();

    return AV_ERR_OK;
}

int32_t RNSkHarmonyVideo::Start() {
    DLOG(INFO) << "RNSkHarmonyVideo Start begin.";
    std::lock_guard<std::mutex> lock(mutex_);
    if (isStarted_) {
        DLOG(ERROR) << "Already started.";
        return AV_ERR_UNKNOWN;
    }
    int32_t ret = videoDecoder_->StartVideoDecoder();
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) << "Decoder start failed";
        return AV_ERR_UNKNOWN;
    }
    isStarted_ = true;
    isPause_ = false;
    // 有无音频
    if (demuxer_->hasAudio()) {
        DLOG(INFO) << "video has audio";
        ret = audioDecoder_->StartAudioDecoder();
        if (ret != AV_ERR_OK) {
            DLOG(ERROR) << "audio Decoder start failed";
            return AV_ERR_UNKNOWN;
        }
        // 音频输入输出播放
        decAudioInputThread_ = std::make_unique<std::thread>(&RNSkHarmonyVideo::DecAudioInputThread, this);
        decAudioOutputThread_ = std::make_unique<std::thread>(&RNSkHarmonyVideo::DecAudioOutputThread, this);
        if (decAudioInputThread_ == nullptr || decAudioOutputThread_ == nullptr) {
            DLOG(ERROR) << "Create thread failed";
            ReleaseAudio();
            return AV_ERR_UNKNOWN;
        }
    } else {
        isAudioEnd_ = true;
    }

    decVideoInputThread_ = std::make_unique<std::thread>(&RNSkHarmonyVideo::DecVideoInputThread, this);
    decVideoOutputThread_ = std::make_unique<std::thread>(&RNSkHarmonyVideo::DecVideoOutputThread, this);
    if (decVideoInputThread_ == nullptr || decVideoOutputThread_ == nullptr) {
        DLOG(ERROR) << "Create thread failed";
        StartRelease();
        return AV_ERR_UNKNOWN;
    }
    doneCond_.notify_all();
    return AV_ERR_OK;
}

void RNSkHarmonyVideo::StartRelease() {
    DLOG(INFO) << "StartRelease 释放 " << isReleased_;
    if (audioRenderer_) {
        OH_AudioRenderer_Stop(audioRenderer_);
    }
    if (!isReleased_) {
        ReleaseAudio();
        ReleaseVideo();
        ReleaseThread();
        DLOG(INFO) << "StartRelease 释放 end";
    }
}

void RNSkHarmonyVideo::ReleaseAudio() {
    DLOG(INFO) << "ReleaseAudio enter.";
    if (audioRenderer_) {
        OH_AudioRenderer_Flush(audioRenderer_); 
        OH_AudioRenderer_Release(audioRenderer_);
        audioRenderer_ = nullptr;
    }
    if (builder_) {
        OH_AudioStreamBuilder_Destroy(builder_);
        builder_ = nullptr;
    }
    if (audioDecoder_ != nullptr) {
        audioDecoder_->Release();
        audioDecoder_.reset();
    }
    if (audioSignal_ != nullptr) {
        delete audioSignal_;
        audioSignal_ = nullptr;
    }
    DLOG(INFO) << "ReleaseAudio release end";
}

void RNSkHarmonyVideo::ReleaseThread() {
    if (decVideoInputThread_ && decVideoInputThread_->joinable()) {
        decVideoInputThread_->detach();
        decVideoInputThread_.reset();
    }
    if (decVideoOutputThread_ && decVideoOutputThread_->joinable()) {
        decVideoOutputThread_->detach();
        decVideoOutputThread_.reset();
    }
    if (decAudioInputThread_ && decAudioInputThread_->joinable()) {
        decAudioInputThread_->detach();
        decAudioInputThread_.reset();
    }
    if (decAudioOutputThread_ && decAudioOutputThread_->joinable()) {
        decAudioOutputThread_->detach();
        decAudioOutputThread_.reset();
    }
}

void RNSkHarmonyVideo::ReleaseVideo() {
    DLOG(INFO) << "Release enter.";
    std::lock_guard<std::mutex> lock(mutex_);
    ReleaseThread();
    // 清空队列
    while (audioSignal_ && !audioSignal_->renderQueue.empty()) {
        audioSignal_->renderQueue.pop();
    }
    if (decVideoInputThread_ && decVideoInputThread_->joinable()) {
        decVideoInputThread_->detach();
        decVideoInputThread_.reset();
    }
    if (decVideoOutputThread_ && decVideoOutputThread_->joinable()) {
        decVideoOutputThread_->detach();
        decVideoOutputThread_.reset();
    }
    if (demuxer_ != nullptr) {
        demuxer_->Release();
        demuxer_.reset();
    }
    if (videoDecoder_ != nullptr) {
        videoDecoder_->Release();
        videoDecoder_.reset();
    }
    if (videoSignal_ != nullptr) {
        delete videoSignal_;
        videoSignal_ = nullptr;
    }
    if (sampleInfo_.inputFd != -1) {
        close(sampleInfo_.inputFd);
        sampleInfo_.inputFd = -1;
    }
    isStop_ = false;
    isStarted_ = false;
    isReleased_ = true;
    isPause_ = false;
    isEndOfFile_ = false;
    isVideoEndOfFile_ = false;

    if (sampleInfo_.PlayDoneCallback) {
        sampleInfo_.PlayDoneCallback(sampleInfo_.playDoneCallbackData);
        DLOG(INFO) << "play end callback";
    }
    doneCond_.notify_all();
    DLOG(INFO) << "Release end.";
}

void RNSkHarmonyVideo::DecVideoInputThread() {
    DLOG(INFO) << "DecInputThread enter";
    int currentLoop = 0;
    bool shouldContinue = true;
    videoDecoderStopStatus.store(false);
    while (shouldContinue) {
        // 检查是否需要停止
        if (SkiaManager::getInstance().getReleaseVideo() || !isStarted_.load()) {
            DLOG(INFO) << "DecInputThread stop requested";
            break;
        }
        
        std::unique_lock<std::mutex> lock(videoSignal_->inputMutex_);
        videoSignal_->videoInputCond_.wait(lock, [this]() {
            return !isPause_.load() && (!isStarted_ || !videoSignal_->inputBufferInfoQueue_.empty());
        });

        // 再次检查状态
        if (!isStarted_.load()) {
            DLOG(INFO) << "DecInputThread stopped by external request";
            break;
        }
        
        if (videoSignal_->inputBufferInfoQueue_.empty()) {
            DLOG(WARNING) << "DecInputThread Buffer queue is empty, waiting...";
            continue;
        }

        CodecBufferInfo bufferInfo = videoSignal_->inputBufferInfoQueue_.front();
        videoSignal_->inputBufferInfoQueue_.pop();
        videoSignal_->inputFrameCount_++;
        lock.unlock();

        demuxer_->ReadSample(reinterpret_cast<OH_AVBuffer *>(bufferInfo.buffer), bufferInfo.attr);
        flags = bufferInfo.attr.flags;

        if (bufferInfo.attr.flags & AVCODEC_BUFFER_FLAGS_EOS) {
            DLOG(INFO) << "DecInputThread Catch EOS, current loop: " << currentLoop << ", total loops: " << loops_.load();
            videoDecoder_->Flush(bufferInfo);
            int32_t targetLoops = loops_.load();
            if (targetLoops == 1) {
                DLOG(INFO) << "DecInputThread Infinite loop, seeking to beginning";
                seek(0);
                currentLoop = 0;
                continue;
            } else {
                DLOG(INFO) << "DecInputThread No loop configured, ending playback";
                shouldContinue = false;
                isVideoEndOfFile_.store(true);
                break;
            }
        } else {
            int32_t ret = videoDecoder_->PushInputData(bufferInfo);
            if (ret != AV_ERR_OK) {
                if (!isStarted_.load() || SkiaManager::getInstance().getReleaseVideo()) {
                    DLOG(INFO) << "DecInputThread Push data failed due to normal shutdown";
                    shouldContinue = false;
                    break;
                } else {
                    DLOG(ERROR) << "DecInputThread Push data failed unexpectedly, ret: " << ret;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
            }
        }
    }
    videoDecoderStopStatus.store(true);
    DLOG(INFO) << "DecInputThread exiting";
}

void RNSkHarmonyVideo::DecVideoOutputThread() {
    if (sampleInfo_.frameRate <= 0) {
        sampleInfo_.frameRate = DEFAULT_FRAME_RATE;
    }
    sampleInfo_.frameInterval = MICROSECOND / sampleInfo_.frameRate;
    DLOG(INFO) << "sampleInfo_.frameInterval: " << sampleInfo_.frameInterval;
    
    int currentLoop = 0;
    bool shouldContinue = true;
    
    while (shouldContinue) {
        if (SkiaManager::getInstance().getReleaseVideo() || !isStarted_.load()) {
            DLOG(INFO) << "DecOutputThread stop requested";
            break;
        }
        thread_local auto lastPushTime = std::chrono::system_clock::now();
        std::unique_lock<std::mutex> lock(videoSignal_->outputMutex_);
        videoSignal_->videoOutputCond_.wait(lock, [this]() {
            return !isPause_.load() && (!isStarted_ || !videoSignal_->outputBufferInfoQueue_.empty());
        });
        if (!isStarted_.load()) {
            DLOG(INFO) << "DecOutputThread stopped by external request";
            break;
        }
        if (videoSignal_->outputBufferInfoQueue_.empty()) {
            DLOG(WARNING) << "DecOutputThread Buffer queue is empty, waiting...";
            continue;
        }
        CodecBufferInfo bufferInfo = videoSignal_->outputBufferInfoQueue_.front();
        videoSignal_->outputBufferInfoQueue_.pop();
        if (bufferInfo.attr.flags & AVCODEC_BUFFER_FLAGS_EOS) {
            DLOG(INFO) << "DecOutputThread Catch EOS, flushing decoder";
            videoDecoder_->Flush(bufferInfo);
            int32_t targetLoops = loops_.load();
            if (targetLoops == 1) {
                DLOG(INFO) << "DecOutputThread Infinite loop, waiting for restart";
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            } else {
                DLOG(INFO) << "DecOutputThread No loop configured, ending playback";
                shouldContinue = false;
                isVideoEndOfFile_.store(true);
                break;
            }
        }
        
        videoSignal_->outputFrameCount_++;
        frameCount = videoSignal_->outputFrameCount_;
        milliseconds = bufferInfo.attr.pts / 1000;
        
        DLOG(INFO) << "DecOutputThread processing frame " << videoSignal_->outputFrameCount_
                   << ", size: " << bufferInfo.attr.size 
                   << ", pts: " << bufferInfo.attr.pts << "us"
                   << ", thread_id: " << std::this_thread::get_id();
        
        lock.unlock();
        
        OH_AVBuffer *Buffer = reinterpret_cast<OH_AVBuffer *>(bufferInfo.buffer);
        nativeBuffer = OH_AVBuffer_GetNativeBuffer(Buffer);
        if (!videoDecoder_) {
            DLOG(INFO) << "DecOutputThread exit videoDecoder_ null";
            break;
        }
        int32_t ret = videoDecoder_->FreeOutputData(bufferInfo.bufferIndex, true);
        if (ret != AV_ERR_OK) {
            DLOG(ERROR) << "DecOutputThread FreeOutputData failed, ret: " << ret;
            if (!isStarted_.load() || SkiaManager::getInstance().getReleaseVideo()) {
                DLOG(INFO) << "DecOutputThread exit due to normal shutdown";
                shouldContinue = false;
                break;
            }
            continue;
        }
        std::this_thread::sleep_until(lastPushTime + std::chrono::microseconds(sampleInfo_.frameInterval));
        lastPushTime = std::chrono::system_clock::now();
    }
    DLOG(INFO) << "DecOutputThread exiting";
    if (videoSignal_) {
        videoSignal_->videoInputCond_.notify_all();
        videoSignal_->videoOutputCond_.notify_all();
    }
}

int32_t RNSkHarmonyVideo::InitAudio() {
    audioDecoder_ = std::make_unique<AudioDecoder>();
    int32_t ret = audioDecoder_->CreateAudioDecoder(sampleInfo_.audioCodecMime);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) << "Create audio decoder failed";
        return AV_ERR_UNKNOWN;
    }
    audioSignal_ = new ADecSignal;
    ret = audioDecoder_->Config(sampleInfo_, audioSignal_);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) << "audio Decoder config failed";
        return AV_ERR_UNKNOWN;
    }
    AudioInitData audioInitData;
    audioInitData.channelCount = sampleInfo_.channelCount;
    audioInitData.samplingRate = sampleInfo_.sampleRate;
    DLOG(INFO) << "audio audioInitData ：" << audioInitData.samplingRate;
    InitAudioPlayer(audioInitData);
    return AV_ERR_OK;
}

void RNSkHarmonyVideo::InitAudioPlayer(AudioInitData audioInitData) {
    DLOG(INFO) << "InitAudioPlayer enter";
    OH_AudioStreamBuilder_Create(&builder_, audioInitData.type);
    // 设置音频采样率
    OH_AudioStreamBuilder_SetSamplingRate(builder_, audioInitData.samplingRate);
    // 设置音频声道
    OH_AudioStreamBuilder_SetChannelCount(builder_, audioInitData.channelCount);
    // 设置音频采样格式
    OH_AudioStreamBuilder_SetSampleFormat(builder_, audioInitData.format);
    // 设置音频流的编码类型
    OH_AudioStreamBuilder_SetEncodingType(builder_, audioInitData.encodingType);
    // 设置输出音频流的工作场景
    OH_AudioStreamBuilder_SetRendererInfo(builder_, audioInitData.usage);

    OH_AudioRenderer_Callbacks callbacks;
    // 配置回调函数
    callbacks.OH_AudioRenderer_OnWriteData = SampleCallback::OnRenderWriteData;
    callbacks.OH_AudioRenderer_OnStreamEvent = SampleCallback::OnRenderStreamEvent;
    callbacks.OH_AudioRenderer_OnInterruptEvent = SampleCallback::OnRenderInterruptEvent;
    callbacks.OH_AudioRenderer_OnError = SampleCallback::OnRenderError;

    // 设置输出音频流的回调
    OH_AudioStreamBuilder_SetRendererCallback(builder_, callbacks, audioSignal_);
    OH_AudioStreamBuilder_GenerateRenderer(builder_, &audioRenderer_);
}

void RNSkHarmonyVideo::InitControlSignal() {
    if (sampleInfo_.frameRate <= 0) {
        sampleInfo_.frameRate = DEFAULT_FRAME_RATE;
    }
    sampleInfo_.frameInterval = MICROSECOND / int64_t(sampleInfo_.frameRate);
    isReleased_ = false;
    isAudioEnd_ = false;
    isVideoEnd_ = false;
    isStop_ = false;
    isPause_ = false;
    isEndOfFile_ = false;
    isVideoEndOfFile_ = false;
    
    // 初始化循环相关状态
    DLOG(INFO) << "InitControlSignal: loops_ = " << loops_.load();
}

void RNSkHarmonyVideo::DecAudioInputThread() {
    while (true) {
        if (!isStarted_) {
            DLOG(ERROR) << "audio Decoder input thread out";
            break;
        }
        if (isStop_) {
            DLOG(ERROR) << "play stop";
            break;
        }
        std::unique_lock<std::mutex> lock(audioSignal_->audioInputMutex_);
        audioSignal_->audioInputCond_.wait(lock, [this]() {
            return !isPause_.load() && (!isStarted_ || !audioSignal_->audioInputBufferInfoQueue_.empty());
        });
        if (!isStarted_) {
            DLOG(ERROR) << "audio Work done, thread out";
            break;
        }
        if (audioSignal_->audioInputBufferInfoQueue_.empty()) {
            DLOG(ERROR) << "audio Buffer queue is empty, continue";
            continue;
        }

        AudioCodecBufferInfo bufferInfo = audioSignal_->audioInputBufferInfoQueue_.front();
        audioSignal_->audioInputBufferInfoQueue_.pop();
        lock.unlock();
        if (videoDecoderStopStatus) {
            DLOG(ERROR) << "audio input stop";
            break;
        }

        demuxer_->ReadAudioSample(bufferInfo.bufferOrigin, bufferInfo.attr);

        int32_t ret = audioDecoder_->PushInputData(bufferInfo);
        if (ret != AV_ERR_OK) {
            DLOG(ERROR) << "audio Push data failed";
            break;
        }

        if (bufferInfo.attr.flags & AVCODEC_BUFFER_FLAGS_EOS) {
            DLOG(ERROR) << "audio Catch EOS, in thread out";
            break;
        }
    }
    DLOG(ERROR) << "DecAudioInputThread thread out";
}

void RNSkHarmonyVideo::DecAudioOutputThread() {
    while (true) {
        if (!isStarted_ || isStop_) {
            DLOG(ERROR) << "play stop";
            break;
        }
        if (audioSignal_->isInterrupt) {
            DLOG(ERROR) << "audio render interrupt. play stop";
            isStop_ = true;
            break;
        }
        std::unique_lock<std::mutex> pauseLock(pauseMutex_);
        pauseLock.unlock();

        std::unique_lock<std::mutex> lock(audioSignal_->audioOutputMutex_);
        audioSignal_->audioOutputCond_.wait(lock, [this]() {
            return !isPause_.load() && (!isStarted_ || !audioSignal_->audioOutputBufferInfoQueue_.empty());
        });
        if (!isStarted_) {
            DLOG(ERROR) << "audio Decoder output thread out done";
            break;
        }
        if (audioSignal_->audioOutputBufferInfoQueue_.empty()) {
            DLOG(ERROR) << "audio out Buffer queue is empty";
            continue;
        }
        AudioCodecBufferInfo bufferInfo = audioSignal_->audioOutputBufferInfoQueue_.front();
        audioSignal_->audioOutputBufferInfoQueue_.pop();
        if (bufferInfo.attr.flags & AVCODEC_BUFFER_FLAGS_EOS) {
            DLOG(ERROR) << "Catch EOS, audio out thread out";
            isEndOfFile_ = true;
            break;
        }
        uint8_t *source = reinterpret_cast<uint8_t *>(OH_AVBuffer_GetAddr(bufferInfo.bufferOrigin));
        for (int i = 0; i < bufferInfo.attr.size; i++) {
            audioSignal_->renderQueue.push(*(source + i));
        }
        lock.unlock();
         if (videoDecoderStopStatus) {
            DLOG(ERROR) << "audio output stop";
            break;
        }
        int32_t ret = audioDecoder_->FreeOutputData(bufferInfo.bufferIndex, false);
        if (ret != AV_ERR_OK) {
            DLOG(ERROR) << "audio Decoder output thread out free";
            break;
        }
        std::unique_lock<std::mutex> lockRender(audioSignal_->renderMutex);
        audioSignal_->renderCond.wait(
            lockRender, [this, bufferInfo]() { return audioSignal_->renderQueue.size() < 5 * bufferInfo.attr.size; });
    }
    DLOG(ERROR) << "DecAudioOutputThread thread out";
    isAudioEnd_ = true;
    OH_AudioRenderer_Stop(audioRenderer_);
}

sk_sp<SkImage> RNSkHarmonyVideo::nextImage(double *timeStamp) {
    DLOG(INFO) << "nextImage enter  转换 第 " << frameCount << " 帧, nativeBuffer: " << nativeBuffer;
    OH_NativeBuffer_Config config;
    if (nativeBuffer) {
        OH_NativeBuffer_GetConfig(nativeBuffer, &config);
        return SkiaOpenGLSurfaceFactory::makeImageFromHardwareBuffer(nativeBuffer);
        DLOG(INFO) << "nextImage OH_NativeBuffer_Config width : " << config.width << " height : " << config.height
                   << " pixelFormat : " << config.format << " usage :" << config.usage << " stride :" << config.stride;
    } else {
        return nullptr;
    }
}

double RNSkHarmonyVideo::duration() {
    DLOG(INFO) << "duration enter  视频总时长（微秒） : " << demuxer_->sampleInfo.duration;
    double duration = static_cast<double>(demuxer_->sampleInfo.duration);
    double durationMs = duration / 1000; // js侧需要的duration是毫秒单位
    return durationMs;
}

double RNSkHarmonyVideo::framerate() {
    DLOG(INFO) << "framerate enter  视频帧率 :" << sampleInfo_.frameRate;
    return sampleInfo_.frameRate;
}

void RNSkHarmonyVideo::seek(double timestamp) {
    int64_t time;
    if (timestamp == 0) {
        DLOG(INFO) << "seek demuxer loop timestamp: " << timestamp;
        time = timestamp;
    } else {
        time = static_cast<int64_t>(timestamp) + milliseconds;
        DLOG(INFO) << "seek enter  跳转时间（毫秒）: " << time << " 当前时间（毫秒）: " << milliseconds;
    }
    DLOG(INFO) << "seek demuxer timestamp: " << timestamp;
    int32_t ret = OH_AVDemuxer_SeekToTime(demuxer_->demuxer, time, OH_AVSeekMode::SEEK_MODE_CLOSEST_SYNC);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) << "seek demuxer loop failed";
        return;
    }
}

float RNSkHarmonyVideo::getRotationInDegrees() {
    float rotation = static_cast<float>(sampleInfo_.rotate);
    DLOG(INFO) << "rotation  视频角度: " << sampleInfo_.rotate;
    return rotation;
}

SkISize RNSkHarmonyVideo::getSize() {
    DLOG(INFO) << "getSize enter  width: " << demuxer_->sampleInfo.videoWidth
               << " height :" << demuxer_->sampleInfo.height;
    return SkISize::Make(demuxer_->sampleInfo.videoWidth, demuxer_->sampleInfo.height);
}

void RNSkHarmonyVideo::play() {
    DLOG(INFO) << "play enter";
    if (!demuxer_ || demuxer_->sampleInfo.duration == 0) {
        return;
    }
    isPause_.store(false);
    videoSignal_->videoInputCond_.notify_all();
    videoSignal_->videoOutputCond_.notify_all();
    audioSignal_->audioOutputCond_.notify_all();
    audioSignal_->audioInputCond_.notify_all();

    if (audioRenderer_) {
        OH_AudioRenderer_Start(audioRenderer_);
    }

    SkiaManager::getInstance().setReleaseVideo(false);
    Start();
    DLOG(INFO) << "play end.";
}

void RNSkHarmonyVideo::pause() {
    DLOG(INFO) << "pause enter.";
    if (audioRenderer_) {
        OH_AudioRenderer_Pause(audioRenderer_);
    }
    isPause_.store(true);
    return;
}

void RNSkHarmonyVideo::setVolume(float volume) {
    DLOG(INFO) << "setVolume enter volume: " << volume;
    OH_AudioRenderer_SetVolume(audioRenderer_, volume);
}

void RNSkHarmonyVideo::setLoop(bool loops) { 
    int32_t oldLoops = loops_.load();
    loops_.store(loops);
    DLOG(INFO) << "SetLoop: changed from " << oldLoops << " to " << loops;
    if (isStarted_.load()) {
        DLOG(INFO) << "SetLoop: Video is running, loop change will take effect on next cycle";
    }
    if (videoSignal_) {
        videoSignal_->videoInputCond_.notify_all();
        videoSignal_->videoOutputCond_.notify_all();
    }
    if (audioSignal_) {
        audioSignal_->audioInputCond_.notify_all();
        audioSignal_->audioOutputCond_.notify_all();
    }
}

void RNSkHarmonyVideo::stop() {
    DLOG(INFO) << "RNSkHarmonyVideo::stop";
    videoDecoderStopStatus = true;
    StartRelease();
}

} // namespace RNSkia
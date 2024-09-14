/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

#undef LOG_TAG
#define LOG_TAG "player"

namespace {
using namespace std::chrono_literals;
}
namespace RNSkia {
RNSkHarmonyVideo RNSkHarmonyVideo::HarmonyVideo;
RNSkHarmonyVideo::~RNSkHarmonyVideo()
{
    RNSkHarmonyVideo::StartRelease();
}

int32_t RNSkHarmonyVideo::OpenFile(SampleInfo &sampleInfo)
{
    DLOG(INFO) <<"OpenFile enter.";
    if (sampleInfo.uri.empty()) {
        DLOG(INFO) <<"OpenFile: url empty";
        return AV_ERR_UNKNOWN;
    }
    
    std::string assetsFilePath = sampleInfo.uri;
    std::string Prefixes = "../../";
    
    if (sampleInfo.uri.find(Prefixes) == 0) {
        assetsFilePath = DEFAULT_ASSETS_DEST + sampleInfo.uri.substr(Prefixes.length());
        DLOG(INFO) << "ReadAssetsData assetsFilePath=" << assetsFilePath;
    }
      
    if (nativeResMgr == nullptr) {
        DLOG(ERROR) << "ReadAssetsData env error, nativeResMgr: "<< nativeResMgr;
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
    DLOG(INFO) <<"Open file: fd = " << sampleInfo.inputFd<<" sampleInfo.inputFileSize: " << sampleInfo.inputFileSize;
    if (sampleInfo.inputFd == -1) {
        DLOG(ERROR) <<"cont open file, error: " << strerror(errno);
        return AV_ERR_UNKNOWN;
    }

    DLOG(INFO) <<"OpenFile end.";
    return AV_ERR_OK;
}

int32_t RNSkHarmonyVideo::Init(SampleInfo &sampleInfo)
{
    DLOG(INFO) << "Init enter.";
    std::lock_guard<std::mutex> lock(mutex_);
    if (isStarted_) {
        DLOG(ERROR) <<"Already started.";
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
    signal = new VDecSignal;
    
//     sampleInfo_.window = PluginManager::GetInstance()->m_window; // PluginManager  OH_NativeWindow

    ret = videoDecoder_->Config(sampleInfo_, signal);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) <<  "Decoder config failed";
        return AV_ERR_UNKNOWN;
    }
    InitControlSignal();
    
    return AV_ERR_OK;
}

int32_t RNSkHarmonyVideo::Start()
{
    DLOG(INFO) << "RNSkHarmonyVideo Start begin.";
    std::lock_guard<std::mutex> lock(mutex_);
    if (isStarted_) {
        DLOG(ERROR) <<  "Already started.";
        return AV_ERR_UNKNOWN;
    }
    int32_t ret = videoDecoder_->StartVideoDecoder();
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) <<  "Decoder start failed";
        return AV_ERR_UNKNOWN;
    }
    isStarted_ = true;
    
    // 有无音频
    if (demuxer_->hasAudio()) {
        DLOG(INFO) <<"video has audio";
        ret = audioDecoder_->StartAudioDecoder();
        if (ret != AV_ERR_OK) {
            DLOG(ERROR) << "audio Decoder start failed";
            return AV_ERR_UNKNOWN;
        }
        // 音频输入输出播放
        decAudioInputThread_ = std::make_unique<std::thread>(&RNSkHarmonyVideo::DecAudioInputThread, this);
        decAudioOutputThread_ = std::make_unique<std::thread>(&RNSkHarmonyVideo::DecAudioOutputThread, this);
        audioPlayerThread_ = std::make_unique<std::thread>(&RNSkHarmonyVideo::AudioPlay, this);
        if (decAudioInputThread_ == nullptr || decAudioOutputThread_ == nullptr || audioPlayerThread_ == nullptr) {
            DLOG(ERROR) << "Create thread failed";
            ReleaseAudio();
            return AV_ERR_UNKNOWN;
        }
    } else {
        isAudioEnd_ = true;
    }
    
    decInputThread_ = std::make_unique<std::thread>(&RNSkHarmonyVideo::DecInputThread, this);
    decOutputThread_ = std::make_unique<std::thread>(&RNSkHarmonyVideo::DecOutputThread, this);
    if (decInputThread_ == nullptr || decOutputThread_ == nullptr) {
        DLOG(ERROR) << "Create thread failed";
        StartRelease();
        return AV_ERR_UNKNOWN;
    }
    doneCond_.notify_all();
    return AV_ERR_OK;
}

void RNSkHarmonyVideo::StartRelease()
{
    if (!isReleased_) {
        isReleased_ = true;
        ReleaseAudio();
        Release();
    }
}

void RNSkHarmonyVideo::ReleaseAudio()
{
    DLOG(INFO) << "ReleaseAudio enter.";
    if (decAudioInputThread_ && decAudioInputThread_->joinable()) {
        decAudioInputThread_->join();
    }
    decAudioInputThread_.reset();

    if (decAudioOutputThread_ && decAudioOutputThread_->joinable()) {
        decAudioOutputThread_->join();
        decAudioOutputThread_.reset();
    }

    if (audioPlayerThread_ && audioPlayerThread_->joinable()) {
        audioPlayerThread_->detach();
        audioPlayerThread_.reset();
    }
    if (audioPlayer_ != nullptr) {
        audioPlayer_->Release();
        audioPlayer_.reset();
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

void RNSkHarmonyVideo::Release()
{
    DLOG(INFO) << "Release enter.";
    std::lock_guard<std::mutex> lock(mutex_);
    isStarted_ = false;
    
    if (decInputThread_ && decInputThread_->joinable()) {
        decInputThread_->detach();
        decInputThread_.reset();
    }
    if (decOutputThread_ && decOutputThread_->joinable()) {
        decOutputThread_->detach();
        decOutputThread_.reset();
    }
    decOutputThread_.reset();
    if (demuxer_ != nullptr) {
        demuxer_->Release();
        demuxer_.reset();
    }

    if (videoDecoder_ != nullptr) {
        videoDecoder_->Release();
        videoDecoder_.reset();
    }
    if (signal != nullptr) {
        delete signal;
        signal = nullptr;
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

    doneCond_.notify_all();
    if (sampleInfo_.PlayDoneCallback) {
        sampleInfo_.PlayDoneCallback(sampleInfo_.playDoneCallbackData);
        DLOG(INFO) <<"play end callback";
    }
    doneCond_.notify_all();
    DLOG(INFO) << "Release end.";
}

void RNSkHarmonyVideo::DecInputThread()
{
    DLOG(INFO) << "DecInputThread enter";
    while (true) {
        if (!isStarted_) {
            DLOG(ERROR) << "Decoder input thread out";
            break;
        }
        std::unique_lock<std::mutex> lock(signal->inputMutex_);
        bool condRet = signal->inputCond_.wait_for(
            lock, 300ms, [this]() { return !isStarted_ || !signal->inputBufferInfoQueue_.empty(); });
        if (!isStarted_) {
            DLOG(ERROR) << "Work done, thread out";
            break;
        }
        if (signal->inputBufferInfoQueue_.empty()) {
            DLOG(ERROR) << "Buffer queue is empty, continue, cond ret: "<< condRet;
        }
        
        CodecBufferInfo bufferInfo = signal->inputBufferInfoQueue_.front();
        signal->inputBufferInfoQueue_.pop();
        signal->inputFrameCount_++;
        lock.unlock();
        
        demuxer_->ReadSample(reinterpret_cast<OH_AVBuffer *>(bufferInfo.buffer), bufferInfo.attr);
        // 送入输入队列进行解码
        int32_t ret = videoDecoder_->PushInputData(bufferInfo);
        if (ret != AV_ERR_OK) {
            DLOG(ERROR) << "Push data failed, thread out";
            break;
        }
        
        if (bufferInfo.attr.flags & AVCODEC_BUFFER_FLAGS_EOS) {
            DLOG(ERROR) << "Catch EOS, thread out";
            break;
        }
    }
}

void RNSkHarmonyVideo::DecOutputThread()
{
    DLOG(INFO) << "DecOutputThread enter";
    if (sampleInfo_.frameRate <= 0) {
        sampleInfo_.frameRate = DEFAULT_FRAME_RATE;
    }
    sampleInfo_.frameInterval = MICROSECOND / sampleInfo_.frameRate;
    DLOG(INFO) << "sampleInfo_.frameInterval:  "<< sampleInfo_.frameInterval;
    while (true) {
        thread_local auto lastPushTime = std::chrono::system_clock::now();
        if (!isStarted_) {
            DLOG(ERROR) << "Decoder output thread out";
            break;
        }
        std::unique_lock<std::mutex> lock(signal->outputMutex_);
        bool condRet = signal->outputCond_.wait_for(
            lock, 300ms, [this]() { return !isStarted_ || !signal->outputBufferInfoQueue_.empty(); });
        if (!isStarted_) {
            DLOG(ERROR) << "Work done, thread out";
            break;
        }
        if (signal->outputBufferInfoQueue_.empty()) {
            DLOG(ERROR) << "Buffer queue is empty, continue, cond ret: " << condRet;
        }
        CodecBufferInfo bufferInfo = signal->outputBufferInfoQueue_.front();
        signal->outputBufferInfoQueue_.pop();
        if (bufferInfo.attr.flags & AVCODEC_BUFFER_FLAGS_EOS) {
            DLOG(ERROR) << "Catch EOS, thread out";
            break;
        }
        signal->outputFrameCount_++;
        frameCount =  signal->outputFrameCount_;
        DLOG(INFO) << "DecOutputThread 输出线程 第 " <<signal->outputFrameCount_ <<" 帧, 大小: "<< bufferInfo.attr.size << 
        " flag: "<<bufferInfo.attr.flags <<" 播放时间标记: "<<  bufferInfo.attr.pts << " 微秒, bufferOrigin: " << bufferInfo.bufferOrigin;
        lock.unlock();
        OH_AVBuffer *Buffer = reinterpret_cast<OH_AVBuffer *>(bufferInfo.buffer);
        nativeBuffer = OH_AVBuffer_GetNativeBuffer(Buffer);

        int32_t ret = videoDecoder_->FreeOutputData(bufferInfo.bufferIndex, true);
        if (ret != AV_ERR_OK) {
            DLOG(ERROR) << "DecOutputThread 解码器输出线程退出";
            break;
        }
        std::this_thread::sleep_until(lastPushTime + std::chrono::microseconds(sampleInfo_.frameInterval));
        lastPushTime = std::chrono::system_clock::now();
    }
    DLOG(ERROR) << "DecOutputThread 线程结束, 当前帧: "<< signal->outputFrameCount_;
}

int32_t RNSkHarmonyVideo::InitAudio()
{
    audioDecoder_ = std::make_unique<AudioDecoder>();
    audioPlayer_ = std::make_unique<AudioPlayer>();
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
    audioPlayer_->Init(audioInitData);
    return AV_ERR_OK;
}

void RNSkHarmonyVideo::InitControlSignal()
{
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
}

void RNSkHarmonyVideo::DecAudioInputThread()
{
    DLOG(INFO) << "DecAudioInputThenter enter";
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
        bool condRet = audioSignal_->audioInputCond_.wait_for(
            lock, 300ms, [this]() { return !isStarted_ || !audioSignal_->audioInputBufferInfoQueue_.empty(); });
        if (!isStarted_) {
            DLOG(ERROR) << "audio Work done, thread out";
            break;
        }
        if (audioSignal_->audioInputBufferInfoQueue_.empty()) {
            DLOG(ERROR) << "audio Buffer queue is empty, continue, cond ret: "<< condRet;
        }
        
        AudioCodecBufferInfo bufferInfo = audioSignal_->audioInputBufferInfoQueue_.front();
        audioSignal_->audioInputBufferInfoQueue_.pop();
        lock.unlock();

        demuxer_->ReadAudioSample(bufferInfo.bufferOrigin, bufferInfo.attr);

        int32_t ret = audioDecoder_->PushInputData(bufferInfo);
        if (ret != AV_ERR_OK) {
            DLOG(ERROR) << "audio Push data failed, thread out";
            break;
        }

        if (bufferInfo.attr.flags & AVCODEC_BUFFER_FLAGS_EOS) {
            DLOG(ERROR) << "audio Catch EOS, in thread out";
            break;
        }
    }
    DLOG(ERROR) << "DecAudioInputThread thread out";
}

void RNSkHarmonyVideo::GetPCMBufferData(AudioCodecBufferInfo bufferInfo)
{
    uint8_t *dataPtr = reinterpret_cast<uint8_t *>(OH_AVBuffer_GetAddr(bufferInfo.bufferOrigin));
    std::vector<uint8_t> oneFrame;
    oneFrame.assign(dataPtr, dataPtr + bufferInfo.attr.size);

    audioPlayer_->InsertBuffer(oneFrame);
}

void RNSkHarmonyVideo::DecAudioOutputThread()
{
    DLOG(INFO) << "DecAudioOutputThread :  "<< ONEK * MICROSECOND / int32_t(sampleInfo_.sampleRate) << 
    " sampleInfo_.frameInterval " << sampleInfo_.frameInterval;
    int32_t audioFrameInterval = ONEK * MICROSECOND / int32_t(sampleInfo_.sampleRate);
    while (true) {
        thread_local auto lastPushTime = std::chrono::system_clock::now();
        if (!isStarted_) {
            DLOG(ERROR) << "audio Decoder output thread out";
            break;
        }
        if (isStop_) {
            DLOG(ERROR) << "play stop";
            break;
        }
        std::unique_lock<std::mutex> lock(audioSignal_->audioOutputMutex_);
        bool condRet = audioSignal_->audioOutputCond_.wait_for(
            lock, 300ms, [this]() { return !isStarted_ || !audioSignal_->audioOutputBufferInfoQueue_.empty(); });
        if (!isStarted_) {
            DLOG(ERROR) << "audio Decoder output thread out done";
            break;
        }
        if (audioSignal_->audioOutputBufferInfoQueue_.empty()) {
            DLOG(ERROR) << "audio out Buffer queue is empty, continue, cond ret: "<< condRet;
        }

        AudioCodecBufferInfo bufferInfo = audioSignal_->audioOutputBufferInfoQueue_.front();
        audioSignal_->audioOutputBufferInfoQueue_.pop();
        if (bufferInfo.attr.flags & AVCODEC_BUFFER_FLAGS_EOS) {
            DLOG(ERROR) << "Catch EOS, audio out thread out";
            isEndOfFile_ = true;
            break;
        }
        lock.unlock();
        GetPCMBufferData(bufferInfo);

        int32_t ret = audioDecoder_->FreeOutputData(bufferInfo.bufferIndex, false);
        if (ret != AV_ERR_OK) {
            DLOG(ERROR) << "audio Decoder output thread out free";
            return;
        }
        std::this_thread::sleep_until(lastPushTime + std::chrono::microseconds(audioFrameInterval));
        lastPushTime = std::chrono::system_clock::now();
    }
    DLOG(ERROR) << "DecAudioOutputThread thread out";
}

void RNSkHarmonyVideo::AudioPlay()
{
    DLOG(INFO) << "AudioPlay start audioPlayer_=" << audioPlayer_;
    audioPlayer_->Start();
    while (true) {
        DLOG(INFO) << "AudioPlay audioPlayer_=" << audioPlayer_;
        if (isEndOfFile_) {
            DLOG(ERROR) << "AudioPlay end of file";
            audioPlayer_->EndOfFile();
            break;
        }
        if (audioPlayer_->IsStop()) {
            DLOG(ERROR) << "AudioPlay stop";
            break;
        }
        if (isStop_) {
            DLOG(ERROR) << "play stop";
            break;
        }
        
        std::unique_lock<std::mutex> pauseLock(pauseMutex_);
        if (isPause_) {
            audioPlayer_->Pause();
            pauseCond_.wait(pauseLock, [this]() { return !isPause_;});
            audioPlayer_->Start();
        }
        pauseLock.unlock();
        std::chrono::milliseconds sleepTime(AUDIO_SLEEP_TIME);
        std::this_thread::sleep_for(sleepTime);
    }

    audioPlayer_->Stop();
    isAudioEnd_ = true;
    StartRelease();
    DLOG(INFO) << "AudioPlay finish audioPlayer_=" << audioPlayer_;
}

void RNSkHarmonyVideo::PauseAndResume()
{
    if (!isStarted_) {
        DLOG(ERROR) << "PauseAndResume player is start";
        return;
    }
    std::lock_guard<std::mutex> lock(pauseMutex_);
    isPause_ = !isPause_;
    pauseCond_.notify_all();
}

sk_sp<SkImage> RNSkHarmonyVideo::nextImage(double *timeStamp)
{
    DLOG(INFO) << "nextImage enter  转换 第 "<< frameCount <<" 帧, nativeBuffer: "<<nativeBuffer;
    if (frameCount > 0) {
        OH_NativeBuffer_Config config;
        if(nativeBuffer) {
            OH_NativeBuffer_GetConfig(nativeBuffer, &config);
        }
        DLOG(INFO) << "nextImage OH_NativeBuffer_Config width : "<<config.width<<" height : "<<config.height
            <<" pixelFormat : "<< config.format<<" usage :"<<config.usage<< " stride :"<<config.stride;
//         int ret = OH_NativeBuffer_Unreference(nativeBuffer);
//         if (ret != AV_ERR_OK) {
//             DLOG(ERROR) << "nextImage OH_NativeBuffer_Unreference failed";
//         }
//         OH_AVBuffer_Destroy(buffer);
        
        return context->makeImageFromNativeBuffer(nativeBuffer);
//         return SkiaOpenGLSurfaceFactory::makeImageFromHardwareBuffer(nativeBuffer);
    }
     return nullptr;
}

double RNSkHarmonyVideo::duration()
{
    DLOG(INFO) << "duration enter  视频总时长（微秒） : "<<demuxer_->sampleInfo.duration;
    double duration = static_cast<double>(demuxer_->sampleInfo.duration);
    return duration;
}

double RNSkHarmonyVideo::framerate()
{
    DLOG(INFO) << "framerate enter  视频帧率 :" << sampleInfo_.frameRate;
    return sampleInfo_.frameRate;
}

void RNSkHarmonyVideo::seek(double timestamp)
{
    DLOG(INFO) << "seek enter  入参时间戳 : " << timestamp;
    int64_t millisecond = static_cast<int64_t>(timestamp * 1000);
    int32_t ret = OH_AVDemuxer_SeekToTime(demuxer_->demuxer, millisecond, OH_AVSeekMode::SEEK_MODE_NEXT_SYNC);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) << "seek demuxer failed";
        return;
    }
}

float RNSkHarmonyVideo::getRotationInDegrees()
{
    float rotation = static_cast<float>(sampleInfo_.rotate);
    DLOG(INFO) << "rotation  视频角度: "<< sampleInfo_.rotate;
    return rotation;
}

SkISize RNSkHarmonyVideo::getSize()
{
    DLOG(INFO) << "getSize enter  width: "<< demuxer_->sampleInfo.videoWidth << " height :" <<demuxer_->sampleInfo.videoHeight;
    return SkISize::Make(demuxer_->sampleInfo.videoWidth, demuxer_->sampleInfo.height);
}

void RNSkHarmonyVideo::play()
{
    DLOG(INFO) << "play enter";
    if (isStarted_) {
        DLOG(ERROR) << "Already started.";
        return;
    }
    Start();
    DLOG(INFO) << "play end.";
}

void RNSkHarmonyVideo::pause()
{
    DLOG(INFO) << "pause enter.";
    PauseAndResume();
}

void RNSkHarmonyVideo::setVolume(float volume)
{
    DLOG(INFO) << "setVolume enter.";
    float a = 200.000000;
    audioPlayer_->SetVolume(a);
    DLOG(INFO) << "setVolume volume:"<< volume;
}

} // namespace skia
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
#include <chrono>
#include <cstdint>
#include <multimedia/player_framework/native_averrors.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>

namespace RNSkia {

RNSkHarmonyVideo::~RNSkHarmonyVideo()
{
    RNSkHarmonyVideo::StartRelease();
}

RNSkHarmonyVideo *RNSkHarmonyVideo::GetInstance()
{
    static RNSkHarmonyVideo GetInstance;
    return &GetInstance;
}

int32_t OpenFile(SampleInfo &sampleInfo)
{
    if (sampleInfo.uri.empty()) {
        DLOG(INFO) <<"OpenFile: url empty";
        return AV_ERR_UNKNOWN;
    }
    const char *file = sampleInfo.uri.c_str();

    sampleInfo.inputFd = -1;
    sampleInfo.inputFileSize = 0;
    sampleInfo.inputFileOffset = 0;
    sampleInfo.inputFd = open(file, O_RDONLY);
    DLOG(INFO) <<"Open file: fd = %{public}d" << sampleInfo.inputFd;
    if (sampleInfo.inputFd == -1) {
        DLOG(ERROR) <<"cont open file";
        return AV_ERR_UNKNOWN;
    }

    struct stat fileStatus {};
    if (stat(file, &fileStatus) == 0) {
        sampleInfo.inputFileSize = static_cast<size_t>(fileStatus.st_size);
        DLOG(INFO) <<"Get stat:%{public}zu" << sampleInfo.inputFileSize;
        return AV_ERR_OK;
    } else {
        DLOG(INFO) <<"Get stat file size error";
        return AV_ERR_UNKNOWN;
    }
}

int32_t RNSkHarmonyVideo::Init(SampleInfo &sampleInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (isStarted_) {
        DLOG(ERROR) <<"Already started.";
        return AV_ERR_UNKNOWN;
    }
    if (OpenFile(sampleInfo) != AV_ERR_OK) {
        return AV_ERR_UNKNOWN;
    }
    videoDecoder_ = std::make_unique<VideoDecoder>();
    if (!videoDecoder_) {
        DLOG(ERROR) << "Create videoDecoder_ failed";
        return AV_ERR_UNKNOWN;
    }

    demuxer_ = std::make_unique<Demuxer>();
    int32_t ret = demuxer_->CreateDemuxer(sampleInfo);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) << "Create demuxer failed";
        return AV_ERR_UNKNOWN;
    }
    
    ret = videoDecoder_->CreateVideoDecoder(sampleInfo.codecMime);
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
    
    signal_ = new VDecSignal;
    ret = videoDecoder_->Config(sampleInfo, signal_);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) << "Decoder config failed";
        return AV_ERR_UNKNOWN;
    }
    
    InitControlSignal();
    return Start();
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
    DLOG(ERROR) << "audio audioInitData %{public}d" << audioInitData.samplingRate;
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

int32_t RNSkHarmonyVideo::Start()
{
    if (isStarted_) {
        DLOG(ERROR) << "Already started.";
        return AV_ERR_UNKNOWN;
    }
    if (!videoDecoder_ || !demuxer_) {
        DLOG(ERROR) << "Please Init first.";
        return AV_ERR_UNKNOWN;
    }
    isStarted_ = true;
    int32_t ret = videoDecoder_->StartVideoDecoder();
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) << "Decoder start failed";
        return AV_ERR_UNKNOWN;
    }
    if (demuxer_->hasAudio()) {
        DLOG(INFO) <<"video has audio";
        ret = audioDecoder_->StartAudioDecoder();
        if (ret != AV_ERR_OK) {
            DLOG(ERROR) << "audio Decoder start failed";
            return AV_ERR_UNKNOWN;
        }
        
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
    return AV_ERR_OK;
}

void RNSkHarmonyVideo::StartRelease()
{
    DLOG(INFO) <<"StartRelease %{public}d"<< isVideoEnd_.load()<< " %{public}d" << isAudioEnd_.load();
    if (isReleased_) return;
    // 获取锁
    std::unique_lock<std::mutex> lock(mutex_);
    if (!isVideoEnd_ || !isAudioEnd_) return;
    // 确保只释放一次
    if (!isReleased_) {
        // 执行释放操作
        ReleaseAudio();
        Release();
    }
}

void RNSkHarmonyVideo::ReleaseAudio()
{
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
    DLOG(ERROR) << "Player: audio release end";
}

void RNSkHarmonyVideo::Release()
{
    if (decInputThread_ && decInputThread_->joinable()) {
        decInputThread_->join();
    }
    decInputThread_.reset();

    if (decOutputThread_ && decOutputThread_->joinable()) {
        decOutputThread_->join();
    }
    decOutputThread_.reset();

    if (renderThread_ && renderThread_->joinable()) {
        renderThread_->detach();
        renderThread_.reset();
    }
    
    if (videoDecoder_ != nullptr) {
        videoDecoder_->Release();
        videoDecoder_.reset();
    }
    if (demuxer_ != nullptr) {
        demuxer_->Release();
        demuxer_.reset();
    }
    
    if (sampleInfo_.inputFd != -1) {
        close(sampleInfo_.inputFd);
        sampleInfo_.inputFd = -1;
    }

    auto emptyQueue = std::queue<std::vector<uint8_t>>();
    renderQueue_.swap(emptyQueue);
    if (signal_ != nullptr) {
        delete signal_;
        signal_ = nullptr;
    }
    
    isStop_ = false;
    isStarted_ = false;
    isReleased_ = true;
    isPause_ = false;
    isEndOfFile_ = false;
    isVideoEndOfFile_ = false;
    renderFrameCurIdx_ = 0;
    if (sampleInfo_.PlayDoneCallback) {
        sampleInfo_.PlayDoneCallback(sampleInfo_.playDoneCallbackData);
        DLOG(INFO) <<"play end callback";
    }
    DLOG(ERROR) << "Player: release end: %{public}d"<< loops_;
}

void RNSkHarmonyVideo::DecInputThread()
{
    while (true) {
        if (!isStarted_) {
            DLOG(ERROR) << "Decoder input thread out";
            break;
        }
        if (isStop_) {
            DLOG(ERROR) << "play stop";
            break;
        }
        std::unique_lock<std::mutex> lock(signal_->inputMutex_);
        bool condRet = signal_->inputCond_.wait_for(
            lock, 300ms, [this]() { return !isStarted_ || !signal_->inputBufferInfoQueue_.empty();});
        if (!isStarted_) {
            DLOG(ERROR) << "Work done, thread out";
            break;
        }
        if (signal_->inputBufferInfoQueue_.empty()) {
            DLOG(ERROR) << "DecInputThread Buffer queue is empty, continue, cond ret: %{public}d"<< condRet;
            continue;
        }

        CodecBufferInfo bufferInfo = signal_->inputBufferInfoQueue_.front();
        signal_->inputBufferInfoQueue_.pop();
        signal_->inputFrameCount_++;
        lock.unlock();

        if (!bufferInfo.buffer) {
            DLOG(ERROR) << "DecInputThread bufferInfo buffer empty, continue";
            continue;
        }
        demuxer_->ReadSample(reinterpret_cast<OH_AVBuffer *>(bufferInfo.buffer), bufferInfo.attr);
        int32_t ret = videoDecoder_->PushInputData(bufferInfo);
        if (ret != AV_ERR_OK) {
            DLOG(ERROR) << "Push data failed, thread out";
            break;
        }
        
        if (bufferInfo.attr.flags & AVCODEC_BUFFER_FLAGS_EOS) {
            DLOG(ERROR) << "Catch EOS, in thread out";
            break;
        }
    }
    DLOG(ERROR) << "DecInputThread thread out";
}

void RNSkHarmonyVideo::GetBufferData(CodecBufferInfo bufferInfo)
{
    std::vector<uint8_t> buffer;
    OH_AVFormat *temFormat = OH_VideoDecoder_GetOutputDescription(bufferInfo.codec);
    OH_AVFormat_GetIntValue(temFormat, OH_MD_KEY_VIDEO_STRIDE, &bytesPerRow_);
    OH_AVFormat_GetIntValue(temFormat, OH_MD_KEY_WIDTH, &sliceWidth_);
    OH_AVFormat_GetIntValue(temFormat, OH_MD_KEY_HEIGHT, &sliceHeight_);
    OH_AVFormat_GetIntValue(temFormat, OH_MD_KEY_PIXEL_FORMAT, &sliceFormat_);
    
    OH_AVFormat_Destroy(temFormat);
    if (!bufferInfo.bufferOrigin)
        return;
    uint8_t *dataPtr = reinterpret_cast<uint8_t *>(OH_AVBuffer_GetAddr(bufferInfo.bufferOrigin));
    buffer.assign(dataPtr, dataPtr + bufferInfo.attr.size);

    std::unique_lock<std::mutex> pauseLock(pauseMutex_);
    pauseCond_.wait(pauseLock, [this]() { return !isPause_;});
    pauseLock.unlock();
    std::unique_lock<std::mutex> lock(renderMutex_);
    renderQueue_.push(buffer);
    renderCond_.notify_all();
}

void RNSkHarmonyVideo::DecOutputThread()
{
    DLOG(ERROR) << "sampleInfo_.frameInterval:  %{public}ld"<< sampleInfo_.frameInterval;
    while (true) {
        thread_local auto lastPushTime = std::chrono::system_clock::now();
        if (!isStarted_) {
            DLOG(ERROR) << "Decoder output thread out";
            break;
        }
        if (isStop_) {
            DLOG(ERROR) << "play stop";
            break;
        }
        std::unique_lock<std::mutex> lock(signal_->outputMutex_);
        bool condRet = signal_->outputCond_.wait_for(
            lock, 300ms, [this]() { return !isStarted_ || !signal_->outputBufferInfoQueue_.empty(); });
        if (!isStarted_) {
            DLOG(ERROR) << "Decoder output thread done out";
            break;
        }
        if (signal_->outputBufferInfoQueue_.empty()) {
            DLOG(ERROR) << "DecOutputThread Buffer queue is empty, continue, cond ret: %{public}d"<< condRet;
            continue;
        }

        CodecBufferInfo bufferInfo = signal_->outputBufferInfoQueue_.front();
        signal_->outputBufferInfoQueue_.pop();
        if (bufferInfo.attr.flags & AVCODEC_BUFFER_FLAGS_EOS) {
            DLOG(ERROR) << "Catch EOS, out thread out";
            isVideoEndOfFile_ = true;
            break;
        }
        
        signal_->outputFrameCount_++;
        DLOG(INFO) <<"Out buffer count: %{public}d" << signal_->outputFrameCount_ << "size: %{public}d" <<
            bufferInfo.attr.size << "flag: %{public}u" <<  bufferInfo.attr.flags << " pts: %{public}ld" <<  bufferInfo.attr.pts;
        lock.unlock();
        GetBufferData(bufferInfo);

        int32_t ret = videoDecoder_->FreeOutputData(bufferInfo.bufferIndex, false);
        if (ret != AV_ERR_OK) {
            DLOG(ERROR) << "Decoder output thread out free";
            break;
        }

        std::this_thread::sleep_until(lastPushTime + std::chrono::microseconds(sampleInfo_.frameInterval));
        lastPushTime = std::chrono::system_clock::now();
    }
    DLOG(ERROR) << "Exit, frame count: %{public}u"<< signal_->outputFrameCount_;
}

void RNSkHarmonyVideo::DecAudioInputThread()
{
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
            DLOG(ERROR) << "audio Buffer queue is empty, continue, cond ret: %{public}d"<< condRet;
            continue;
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
    DLOG(ERROR) << "DecAudioOutputThread :%{public}ld %{public}ld "<<
        ONEK * MICROSECOND / int32_t(sampleInfo_.sampleRate)<< "%{public}ld" << sampleInfo_.frameInterval;
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
            DLOG(ERROR) << "audio out Buffer queue is empty, continue, cond ret: %{public}d"<< condRet;
            continue;
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
            break;
        }
        std::this_thread::sleep_until(lastPushTime + std::chrono::microseconds(audioFrameInterval));
        lastPushTime = std::chrono::system_clock::now();
    }
    DLOG(ERROR) << "DecAudioOutputThread thread out";
}

void RNSkHarmonyVideo::AudioPlay()
{
    audioPlayer_->Start();
    while (true) {
        if (isEndOfFile_) {
            DLOG(ERROR) << "AudioPlay end of file";
            audioPlayer_->EndOfFile();
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
}

sk_sp<SkImage> RNSkHarmonyVideo::nextImage(double *timeStamp)
{
    seek(*timeStamp);
    OH_NativeBuffer_Config config = {};
    config.width = sliceWidth_;
    config.height = sliceHeight_;
    config.format = sliceFormat_;
    config.stride = bytesPerRow_;
    OH_NativeBuffer *buffer = OH_NativeBuffer_Alloc(&config);
    if (buffer == nullptr) {
        return 0;
    }
    return context->makeImageFromNativeBuffer(buffer);
}

double RNSkHarmonyVideo::duration()
{
    double duration = static_cast<double>(sampleInfo_.durationTime * 1000);
    return duration;
}

double RNSkHarmonyVideo::framerate()
{
    return sampleInfo_.frameRate;
}

void RNSkHarmonyVideo::seek(double timestamp)
{
    int64_t millisecond = static_cast<int64_t>(timestamp * 1000);
    int32_t ret = OH_AVDemuxer_SeekToTime(demuxer_->demuxer, millisecond, OH_AVSeekMode::SEEK_MODE_NEXT_SYNC);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) << "seek demuxer failed";
        return;
    }
}

float RNSkHarmonyVideo::getRotationInDegrees()
{
    float rotation_ = static_cast<float>(sampleInfo_.rotation);
    return rotation_;
}

SkISize RNSkHarmonyVideo::getSize()
{
    return SkISize::Make(sampleInfo_.width, sampleInfo_.height);
}

void RNSkHarmonyVideo::play()
{
    if (isStarted_) {
        DLOG(ERROR) << "Already started.";
        return;
    }
    RNSkHarmonyVideo *GetInstance = RNSkHarmonyVideo::GetInstance();
    sampleInfo_.uri = url;
    GetInstance->Init(sampleInfo_);
}

void RNSkHarmonyVideo::pause()
{
    RNSkHarmonyVideo *GetInstance = RNSkHarmonyVideo::GetInstance();
    GetInstance->PauseAndResume();
}

void RNSkHarmonyVideo::setVolume(float volume)
{
   audioPlayer_->SetVolume(volume);
}

} // namespace RNSkia


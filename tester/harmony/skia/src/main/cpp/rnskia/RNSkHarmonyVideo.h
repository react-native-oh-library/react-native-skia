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

#ifndef RN_SK_HARMONY_VIDEO_H
#define RN_SK_HARMONY_VIDEO_H

#include <rawfile/raw_file_manager.h>
#include <string>
#include "RNSkVideo.h"
#include "audio_decoder.h"
#include "audio_player.h"
#include "RNSkPlatformContext.h"

#include <bits/alltypes.h>
#include <mutex>
#include <memory>
#include <atomic>
#include <thread>
#include <unistd.h>

#include "decoder.h"
#include "demuxer.h"
#include "sample_info.h"


namespace RNSkia {

class RNSkHarmonyVideo : public RNSkVideo {
public:
    RNSkHarmonyVideo() {}
    static RNSkHarmonyVideo *GetInstance()
    {
        return &RNSkHarmonyVideo::HarmonyVideo;
    }
    
    RNSkHarmonyVideo(std::string url, RNSkPlatformContext *context, const NativeResourceManager *nativeResourceManager)
    : URI(url), context(context) ,nativeResMgr(nativeResourceManager) {
        this->nativeResMgr = nativeResourceManager;
        sampleInfo_.uri = URI;
        Init(sampleInfo_);
    }
    ~RNSkHarmonyVideo();
    
    bool IsRunning() { return isStarted_; }
    
    int32_t OpenFile(SampleInfo &sampleInfo);
    int32_t Init(SampleInfo &sampleInfo);
    int32_t Start();
    void StartRelease();
    void Release();
    void ReleaseAudio();
    void AudioPlay();
    void PauseAndResume();
    
    RNSkPlatformContext *context;
    const NativeResourceManager *nativeResMgr;
    std::string DEFAULT_ASSETS_DEST = "assets/";
    std::string DEFAULT_HTTP_DEST = "http";
private:
    
    int32_t frameCount = 0;
    OH_NativeBuffer *nativeBuffer = nullptr;
    
    static RNSkHarmonyVideo HarmonyVideo;
    
    std::mutex pauseMutex_;
    std::condition_variable pauseCond_;
    bool isPause_{false};
    
    void DecAudioInputThread();
    void DecAudioOutputThread();

    void DecInputThread();
    void DecOutputThread();
    void RenderThread();
    void GetBufferData(CodecBufferInfo bufferInfo);
    
    int32_t InitAudio();
    void InitControlSignal();
    void GetPCMBufferData(AudioCodecBufferInfo bufferInfo);
    
    std::string URI;
    std::unique_ptr<VideoDecoder> videoDecoder_ = nullptr;
    std::unique_ptr<Demuxer> demuxer_ = nullptr;
    std::unique_ptr<AudioDecoder> audioDecoder_ = nullptr;
    std::unique_ptr<AudioPlayer> audioPlayer_ = nullptr;
    
    
    ADecSignal *audioSignal_ = nullptr;
    VDecSignal *signal_ = nullptr;

    std::atomic<bool> isStop_{false};
    std::atomic<bool> isVideoEnd_{false};
    std::atomic<bool> isAudioEnd_{false};
    std::atomic<bool> isEndOfFile_{false};
    std::atomic<bool> isVideoEndOfFile_{false};

    std::unique_ptr<std::thread> decAudioInputThread_ = nullptr;
    std::unique_ptr<std::thread> decAudioOutputThread_ = nullptr;
    std::unique_ptr<std::thread> audioPlayerThread_ = nullptr;
    
    static constexpr int32_t DEFAULT_FRAME_RATE = 30;
    static constexpr int32_t ONEK = 1024;
    static constexpr int32_t AUDIO_SLEEP_TIME = 300;
    
    std::mutex mutex_;
    std::atomic<bool> isStarted_{false};
    std::atomic<bool> isReleased_{false};
    std::unique_ptr<std::thread> decInputThread_ = nullptr;
    std::unique_ptr<std::thread> decOutputThread_ = nullptr;
    std::condition_variable doneCond_;
    SampleInfo sampleInfo_;
    VDecSignal *signal = nullptr;
    static constexpr int64_t MICROSECOND = 1000000;
    
public:
    
    sk_sp<SkImage> nextImage(double *timeStamp = nullptr) override;
    double duration() override;
    double framerate() override;
    void seek(double timestamp) override;
    float getRotationInDegrees() override;
    SkISize getSize() override;
    void play() override;
    void pause() override;
    void setVolume(float volume) override;
    
};

} // namespace RNSkia

#endif // RN_SK_HARMONY_VIDEO_H





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

#include <string>
#include "RNSkVideo.h"

#include "RNSkPlatformContext.h"

#include <mutex>
#include <memory>
#include <atomic>
#include <thread>
#include <unistd.h>
#include "audio_decoder.h"
#include "audio_player.h"
#include "decoder.h"
#include "demuxer.h"
#include "sample_info.h"
#include <glog/logging.h>

namespace RNSkia {

using namespace std::chrono_literals;

class RNSkHarmonyVideo : public RNSkVideo {
public:
    std::string url;

    RNSkHarmonyVideo() = default;
    RNSkHarmonyVideo(std::string url, RNSkPlatformContext *context) : url(std::move(url)), context(context) {}

    ~RNSkHarmonyVideo();

    RNSkHarmonyVideo *GetInstance();
    RNSkPlatformContext *context;
    int32_t Init(SampleInfo &sampleInfo);
    int32_t Start();

    void PauseAndResume() {
        if (!isStarted_) {
            DLOG(ERROR) << "PauseAndResume player is start";
            return;
        }
        std::lock_guard<std::mutex> lock(pauseMutex_);
        isPause_ = !isPause_;
        pauseCond_.notify_all();
    }

    bool IsRunning() { return isStarted_; }

    void StartRelease();

private:
    int32_t loops_ = 1;

    std::mutex pauseMutex_;
    std::condition_variable pauseCond_;
    bool isPause_{false};

    void Release();
    void ReleaseAudio();
    void DecInputThread();
    void DecOutputThread();
    void DecAudioInputThread();
    void DecAudioOutputThread();
    void AudioPlay();

    int32_t InitAudio();
    void InitControlSignal();

    void GetBufferData(CodecBufferInfo bufferInfo);
    void GetPCMBufferData(AudioCodecBufferInfo bufferInfo);

    std::mutex renderMutex_;
    std::queue<std::vector<uint8_t>> renderQueue_;
    std::unique_ptr<std::thread> renderThread_ = nullptr;
    std::condition_variable renderCond_;

    std::unique_ptr<Demuxer> demuxer_ = nullptr;
    std::unique_ptr<VideoDecoder> videoDecoder_ = nullptr;
    std::unique_ptr<AudioDecoder> audioDecoder_ = nullptr;
    std::unique_ptr<AudioPlayer> audioPlayer_ = nullptr;
    ADecSignal *audioSignal_ = nullptr;
    VDecSignal *signal_ = nullptr;

    std::atomic<bool> isStop_{false};
    std::atomic<bool> isVideoEnd_{false};
    std::atomic<bool> isAudioEnd_{false};
    std::atomic<bool> isStarted_{false};
    std::atomic<bool> isReleased_{false};
    std::atomic<bool> isEndOfFile_{false};
    std::atomic<bool> isVideoEndOfFile_{false};

    std::unique_ptr<std::thread> decAudioInputThread_ = nullptr;
    std::unique_ptr<std::thread> decAudioOutputThread_ = nullptr;
    std::unique_ptr<std::thread> audioPlayerThread_ = nullptr;

    std::mutex mutex_;
    std::unique_ptr<std::thread> decInputThread_ = nullptr;
    std::unique_ptr<std::thread> decOutputThread_ = nullptr;
    SampleInfo sampleInfo_;

    static constexpr int64_t MICROSECOND = 1000000;
    static constexpr int32_t DEFAULT_FRAME_RATE = 30;
    static constexpr int32_t ONEK = 1024;
    static constexpr int32_t AUDIO_SLEEP_TIME = 300;

    int32_t bytesPerRow_ = 0;
    int32_t sliceWidth_ = 0;
    int32_t sliceHeight_ = 0;
    int32_t sliceFormat_ = 0;

    int32_t renderFrameCurIdx_ = 0;

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
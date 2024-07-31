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

#ifndef HARMONY_AUDIO_DECODER_H
#define HARMONY_AUDIO_DECODER_H

#include "multimedia/player_framework/native_avcodec_videodecoder.h"
#include "multimedia/player_framework/native_avbuffer_info.h"
#include "sample_info.h"

namespace RNSkia {
class AudioDecoder {
public:
    AudioDecoder() = default;
    ~AudioDecoder();

    int32_t CreateAudioDecoder(const std::string &codecMime);
    int32_t ConfigureAudioDecoder(const SampleInfo &sampleInfo);
    int32_t Config(const SampleInfo &sampleInfo, ADecSignal *signal);
    int32_t StartAudioDecoder();
    int32_t PushInputData(AudioCodecBufferInfo &info);
    int32_t FreeOutputData(uint32_t bufferIndex, bool render);
    int32_t Release();

private:
    int32_t SetCallback(ADecSignal *signal);

    bool isAVBufferMode_ = false;
    OH_AVCodec *decoder;
};
} // namespace RNSkia
#endif // HARMONY_AUDIO_DECODER_H
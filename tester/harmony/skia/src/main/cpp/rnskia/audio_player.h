/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HARMONY_AUDIO_PLAYER_H
#define HARMONY_AUDIO_PLAYER_H

#include <mutex>
#include <ohaudio/native_audiorenderer.h>
#include <ohaudio/native_audiostream_base.h>
#include <string>

namespace RNSkia {
struct AudioControlData {
    std::mutex mutex_;
    std::vector<uint8_t> buffer;
    std::vector<uint8_t> tmpBuffer;
    int32_t curIndex = 0;
    bool isStop = false;
    bool isEOS = false;
};

struct AudioInitData {
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    int32_t samplingRate = 48000;
    int32_t channelCount = 2;
    OH_AudioStream_SampleFormat format = AUDIOSTREAM_SAMPLE_S16LE;
    OH_AudioStream_EncodingType encodingType = AUDIOSTREAM_ENCODING_TYPE_RAW;
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_MUSIC;
};

class AudioPlayer {
public:
    AudioPlayer() {};
    ~AudioPlayer();
    void Release();
    void SetBuffer(std::vector<uint8_t> buffer);
    void InsertBuffer(std::vector<uint8_t> buffer);

    void SetVolume(float volume);
    
    void Init(AudioInitData audioInitData);
    void Start() { OH_AudioRenderer_Start(audioRenderer_); }
    void Stop() { OH_AudioRenderer_Stop(audioRenderer_); }
    void Pause() { OH_AudioRenderer_Pause(audioRenderer_); }
    
    void EndOfFile() { audioControlData_.isEOS = true; }
    bool IsStop() {return audioControlData_.isStop;}
    
    void GetState(OH_AudioStream_State *state) {OH_AudioRenderer_GetCurrentState(audioRenderer_, state);}
private:
    AudioControlData audioControlData_;
    AudioInitData audioInitData_;
    OH_AudioRenderer *audioRenderer_;
    OH_AudioStreamBuilder *builder_;
    std::string uri_;
};

} // namespace RNSkia
#endif // HARMONY_AUDIO_PLAYER_H

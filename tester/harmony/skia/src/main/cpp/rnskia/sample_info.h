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

#ifndef SAMPLE_INFO_H
#define SAMPLE_INFO_H

#include <EGL/eglplatform.h>
#include <bits/alltypes.h>
#include <cstdint>
#include <js_native_api_types.h>
#include <multimedia/player_framework/native_avcodec_videoencoder.h>
#include <string>
#include <condition_variable>
#include <queue>
#include <native_buffer/native_buffer.h>
#include "multimedia/player_framework/native_avcodec_base.h"
#include "multimedia/player_framework/native_avbuffer.h"

namespace RNSkia {

const std::string_view MIME_VIDEO_AVC = "video/avc";
const std::string_view MIME_VIDEO_HEVC = "video/hevc";

constexpr int32_t BITRATE_10M = 10 * 1024 * 1024; // 10Mbps
constexpr int32_t BITRATE_20M = 20 * 1024 * 1024; // 20Mbps
constexpr int32_t BITRATE_30M = 30 * 1024 * 1024; // 30Mbps

struct SampleInfo {
    int32_t sampleId = 0;
    
    int32_t inputFd = -1;
    int32_t outFd = -1;
    int64_t inputFileOffset = 0;
    int64_t inputFileSize = 0;
    std::string inputFilePath;
    std::string outputFilePath;
    std::string codecMime = MIME_VIDEO_AVC.data();
    int32_t videoWidth = 0;
    int32_t videoHeight = 0;
    double frameRate = 0.0;
    int64_t bitrate = 10 * 1024 * 1024; // 10Mbps;

    int64_t audioBitrate;
    uint32_t sampleRate;
    uint32_t channelCount;
    std::string audioCodecMime;

    int64_t frameInterval = 0;
    int32_t perfmode = 0;
    int32_t durationTime = 0;
    uint32_t maxFrames = UINT32_MAX;
    int32_t isHDRVivid = 0;
    uint32_t repeatTimes = 1;
    OH_AVPixelFormat pixelFormat = AV_PIXEL_FORMAT_NV12; // AV_PIXEL_FORMAT_YUVI420;
    bool needDumpOutput = false;
    uint32_t bitrateMode = CBR;
    int32_t hevcProfile = HEVC_PROFILE_MAIN;
    int32_t rotation = 0;
    NativeWindow* window = nullptr;

    uint32_t bufferSize = 0;
    double readTime = 0;
    double memcpyTime = 0;
    double writeTime = 0;
    
    void (*PlayDoneCallback)(void *context) = nullptr;
    void *playDoneCallbackData = nullptr;
    
    int32_t width;
    int32_t height;
    std::string uri;
};

struct CodecBufferInfo {
    uint32_t bufferIndex = 0;
    uintptr_t *buffer = nullptr;
    uint8_t *bufferAddr = nullptr;
    OH_AVCodec *codec = nullptr;
    OH_AVBuffer *bufferOrigin = nullptr;
    OH_AVCodecBufferAttr attr = {0, 0, 0, AVCODEC_BUFFER_FLAGS_NONE};

    CodecBufferInfo(uint8_t *addr) : bufferAddr(addr){};
    CodecBufferInfo(uint8_t *addr, int32_t bufferSize)
        : bufferAddr(addr), attr({0, bufferSize, 0, AVCODEC_BUFFER_FLAGS_NONE}){};
    CodecBufferInfo(uint32_t argBufferIndex, OH_AVMemory *argBuffer, OH_AVCodecBufferAttr argAttr)
        : bufferIndex(argBufferIndex), buffer(reinterpret_cast<uintptr_t *>(argBuffer)), attr(argAttr){};
    CodecBufferInfo(uint32_t argBufferIndex, OH_AVMemory *argBuffer)
        : bufferIndex(argBufferIndex), buffer(reinterpret_cast<uintptr_t *>(argBuffer)){};
    CodecBufferInfo(uint32_t argBufferIndex, OH_AVBuffer *argBuffer)
        : bufferIndex(argBufferIndex), buffer(reinterpret_cast<uintptr_t *>(argBuffer))
    {
        OH_AVBuffer_GetBufferAttr(argBuffer, &attr);
    };

    CodecBufferInfo(uint32_t argBufferIndex, OH_AVBuffer *argBuffer, OH_AVCodec *argCodec)
        : bufferIndex(argBufferIndex), bufferOrigin(argBuffer), codec(argCodec)
    {
        OH_AVBuffer_GetBufferAttr(argBuffer, &attr);
    };
};

struct AudioCodecBufferInfo {
    uint32_t bufferIndex = 0;
    uint8_t *bufferAddr = nullptr;
    OH_AVCodec *codec = nullptr;
    OH_AVBuffer *bufferOrigin = nullptr;
    OH_AVCodecBufferAttr attr = {0, 0, 0, AVCODEC_BUFFER_FLAGS_NONE};

    AudioCodecBufferInfo(uint32_t argBufferIndex, OH_AVBuffer *argBuffer, OH_AVCodec *argCodec)
        : bufferIndex(argBufferIndex), bufferOrigin(argBuffer), codec(argCodec)
    {
        OH_AVBuffer_GetBufferAttr(argBuffer, &attr);
    };
};

class ADecSignal {
public:
    SampleInfo *sampleInfo = nullptr;

    std::mutex audioInputMutex_;
    std::condition_variable audioInputCond_;
    std::queue<AudioCodecBufferInfo> audioInputBufferInfoQueue_;
    std::mutex audioOutputMutex_;
    std::condition_variable audioOutputCond_;
    std::queue<AudioCodecBufferInfo> audioOutputBufferInfoQueue_;

    void ClearQueue()
    {
        {
            std::unique_lock<std::mutex> lock(audioInputMutex_);
            auto emptyQueue = std::queue<AudioCodecBufferInfo>();
            audioInputBufferInfoQueue_.swap(emptyQueue);
        }
        {
            std::unique_lock<std::mutex> lock(audioOutputMutex_);
            auto emptyQueue = std::queue<AudioCodecBufferInfo>();
            audioOutputBufferInfoQueue_.swap(emptyQueue);
        }
    }
};
class VDecSignal {
public:
    SampleInfo *sampleInfo = nullptr;

    uint32_t inputFrameCount_ = 0;
    std::mutex inputMutex_;
    std::condition_variable inputCond_;
    std::queue<CodecBufferInfo> inputBufferInfoQueue_;

    uint32_t outputFrameCount_ = 0;
    std::mutex outputMutex_;
    std::condition_variable outputCond_;
    std::queue<CodecBufferInfo> outputBufferInfoQueue_;

    void ClearQueue()
    {
        {
            std::unique_lock<std::mutex> lock(inputMutex_);
            auto emptyQueue = std::queue<CodecBufferInfo>();
            inputBufferInfoQueue_.swap(emptyQueue);
        }
        {
            std::unique_lock<std::mutex> lock(outputMutex_);
            auto emptyQueue = std::queue<CodecBufferInfo>();
            outputBufferInfoQueue_.swap(emptyQueue);
        }
    }
};

} // namespace RNSkia
#endif // SAMPLE_INFO_H
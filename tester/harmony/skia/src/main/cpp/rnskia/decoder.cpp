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

#include <multimedia/player_framework/native_avcodec_videodecoder.h>
#include <multimedia/player_framework/native_averrors.h>
#include "decoder.h"
#include <glog/logging.h>

#undef LOG_TAG
#define LOG_TAG "VideoDecoder"

namespace RNSkia {
void OnCodecError(OH_AVCodec *codec, int32_t errorCode, void *userData)
{
    (void)codec;
    (void)errorCode;
    (void)userData;
    DLOG(ERROR) <<"On codec error, error code: %{public}d" << errorCode;
}

void OnCodecFormatChange(OH_AVCodec *codec, OH_AVFormat *format, void *userData)
{
    DLOG(ERROR) <<"On codec format change";
}

void OnNeedInputBuffer(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer, void *userData)
{
    if (userData == nullptr) {
        return;
    }
    (void)codec;
    VDecSignal *m_signal = static_cast<VDecSignal *>(userData);
    std::unique_lock<std::mutex> lock(m_signal->inputMutex_);
    m_signal->inputBufferInfoQueue_.emplace(index, buffer);
    m_signal->inputCond_.notify_all();
}

void OnNewOutputBuffer(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer, void *userData)
{
    if (userData == nullptr) {
        return;
    }
    (void)codec;
    VDecSignal *m_signal = static_cast<VDecSignal *>(userData);
    std::unique_lock<std::mutex> lock(m_signal->outputMutex_);
    m_signal->outputBufferInfoQueue_.emplace(index, buffer, codec);
    m_signal->outputCond_.notify_all();
}

VideoDecoder::~VideoDecoder()
{
    Release();
}

int32_t VideoDecoder::CreateVideoDecoder(const std::string &codecMime)
{
    decoder_ = OH_VideoDecoder_CreateByMime(codecMime.c_str());
    if (decoder_ == nullptr) {
        DLOG(ERROR) <<"create decoder_ failed";
        return AV_ERR_UNKNOWN;
    }
    return AV_ERR_OK;
}

int32_t VideoDecoder::SetCallback(VDecSignal *signal)
{
    int32_t ret = AV_ERR_OK;
    ret = OH_VideoDecoder_RegisterCallback(decoder_,
                                           {OnCodecError, OnCodecFormatChange,
                                            OnNeedInputBuffer, OnNewOutputBuffer},
                                           signal);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) <<"Set callback failed, ret: %{public}d" << ret;
        return AV_ERR_UNKNOWN;
    }
    return AV_ERR_OK;
}

int32_t VideoDecoder::ConfigureVideoDecoder(const SampleInfo &sampleInfo)
{
    OH_AVFormat *format = OH_AVFormat_Create();
    if (format == nullptr) {
        DLOG(ERROR) <<"AVFormat create failed";
        return AV_ERR_UNKNOWN;
    }
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_WIDTH, sampleInfo.videoWidth);
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_HEIGHT, sampleInfo.videoHeight);
    OH_AVFormat_SetDoubleValue(format, OH_MD_KEY_FRAME_RATE, sampleInfo.frameRate);
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_PIXEL_FORMAT, sampleInfo.pixelFormat);
    DLOG(ERROR) <<"VideoDecoder config: %{public}d" << sampleInfo.videoWidth << "*%{public}d"<< sampleInfo.videoHeight
        << "%{public}.1f fps" << sampleInfo.frameRate;
    int ret = OH_VideoDecoder_Configure(decoder_, format);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) <<"Config failed, ret: %{public}d" << ret; // 9 AV_ERR_UNSUPPORT
        return AV_ERR_UNKNOWN;
    }
    OH_AVFormat_Destroy(format);
    format = nullptr;

    return AV_ERR_OK;
}

int32_t VideoDecoder::Config(const SampleInfo &sampleInfo, VDecSignal *signal)
{
    if (decoder_ == nullptr) {
        DLOG(ERROR) <<"Decoder is null";
        return AV_ERR_UNKNOWN;
    }
    if (signal == nullptr) {
        DLOG(ERROR) <<"Invalid param: codecUserData";
        return AV_ERR_UNKNOWN;
    }

    // Configure video decoder_
    int32_t ret = ConfigureVideoDecoder(sampleInfo);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) <<"Configure failed";
        return AV_ERR_UNKNOWN;
    }

    // SetCallback for video decoder_
    ret = SetCallback(signal);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) <<"Set callback failed, ret: %{public}d" << ret;
        return AV_ERR_UNKNOWN;
    }

    // Prepare video decoder_
    {
        int ret = OH_VideoDecoder_Prepare(decoder_);
        if (ret != AV_ERR_OK) {
            DLOG(ERROR) <<"Prepare failed, ret: %{public}d" << ret;
            return AV_ERR_UNKNOWN;
        }
    }

    return AV_ERR_OK;
}

int32_t VideoDecoder::StartVideoDecoder()
{
    if (decoder_ == nullptr) {
        DLOG(ERROR) <<"Decoder is null";
        return AV_ERR_UNKNOWN;
    }

    int ret = OH_VideoDecoder_Start(decoder_);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) <<"Start failed, ret: %{public}d" << ret;
        return AV_ERR_UNKNOWN;
    }
    return AV_ERR_OK;
}

int32_t VideoDecoder::PushInputData(CodecBufferInfo &info)
{
    if (decoder_ == nullptr) {
        DLOG(ERROR) <<"Decoder is null";
        return AV_ERR_UNKNOWN;
    }
    int32_t ret = OH_AVBuffer_SetBufferAttr(reinterpret_cast<OH_AVBuffer *>(info.buffer), &info.attr);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) <<"Set avbuffer attr failed";
        return AV_ERR_UNKNOWN;
    }
    ret = OH_VideoDecoder_PushInputBuffer(decoder_, info.bufferIndex);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) <<"Push input data failed";
        return AV_ERR_UNKNOWN;
    }
    return AV_ERR_OK;
}

int32_t VideoDecoder::FreeOutputData(uint32_t bufferIndex, bool render)
{
    if (decoder_ == nullptr) {
        DLOG(ERROR) <<"Decoder is null";
        return AV_ERR_UNKNOWN;
    }

    int32_t ret = AV_ERR_OK;
    if (render) {
        ret = OH_VideoDecoder_RenderOutputBuffer(decoder_, bufferIndex);
    } else {
        ret = OH_VideoDecoder_FreeOutputBuffer(decoder_, bufferIndex);
    }
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) <<"Free output data failed";
        return AV_ERR_UNKNOWN;
    }
    return AV_ERR_OK;
}

int32_t VideoDecoder::Release()
{
    if (decoder_ != nullptr) {
        OH_VideoDecoder_Flush(decoder_);
        OH_VideoDecoder_Stop(decoder_);
        OH_VideoDecoder_Destroy(decoder_);
        decoder_ = nullptr;
    }
    return AV_ERR_OK;
}

} // namespace RNSkia
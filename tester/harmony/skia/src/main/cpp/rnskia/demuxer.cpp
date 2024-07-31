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

#include "demuxer.h"
#include <multimedia/player_framework/native_averrors.h>
#include <sys/stat.h>
#include <glog/logging.h>

namespace RNSkia {

Demuxer::~Demuxer()
{
    Release();
}

int32_t Demuxer::CreateDemuxer(SampleInfo &info)
{
    source = OH_AVSource_CreateWithFD(info.inputFd, info.inputFileOffset, info.inputFileSize);
    if (source == nullptr) {
        DLOG(ERROR) << "create source failed";
        return AV_ERR_UNKNOWN;
    }

    demuxer = OH_AVDemuxer_CreateWithSource(source);
    if (demuxer == nullptr) {
        DLOG(ERROR) << "create demuxer failed";
        return AV_ERR_UNKNOWN;
    }

    auto sourceFormat = std::shared_ptr<OH_AVFormat>(OH_AVSource_GetSourceFormat(source), OH_AVFormat_Destroy);
    if (sourceFormat == nullptr) {
        DLOG(ERROR) << "get source format failed";
        return AV_ERR_UNKNOWN;
    }

    int32_t ret = GetTrackInfo(sourceFormat, info);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) << "get track info failed";
        return AV_ERR_UNKNOWN;
    }
    return AV_ERR_OK;
}

int32_t Demuxer::GetTrackInfo(std::shared_ptr<OH_AVFormat> sourceFormat, SampleInfo &info)
{
    int32_t trackCount = 0;
    OH_AVFormat_GetIntValue(sourceFormat.get(), OH_MD_KEY_TRACK_COUNT, &trackCount);
    for (int32_t index = 0; index < trackCount; index++) {
        int trackType = -1;
        auto trackFormat = std::shared_ptr<OH_AVFormat>(OH_AVSource_GetTrackFormat(source, index), OH_AVFormat_Destroy);
        OH_AVFormat_GetIntValue(trackFormat.get(), OH_MD_KEY_TRACK_TYPE, &trackType);
        if (trackType == MEDIA_TYPE_VID) {
            OH_AVDemuxer_SelectTrackByID(demuxer, index);
            OH_AVFormat_GetIntValue(trackFormat.get(), OH_MD_KEY_WIDTH, &info.videoWidth);
            OH_AVFormat_GetIntValue(trackFormat.get(), OH_MD_KEY_HEIGHT, &info.videoHeight);
            OH_AVFormat_GetDoubleValue(trackFormat.get(), OH_MD_KEY_FRAME_RATE, &info.frameRate);
            OH_AVFormat_GetLongValue(trackFormat.get(), OH_MD_KEY_BITRATE, &info.bitrate);
            OH_AVFormat_GetIntValue(trackFormat.get(), "video_is_hdr_vivid", &info.isHDRVivid);
            OH_AVFormat_GetIntValue(trackFormat.get(), OH_MD_KEY_ROTATION, &info.rotation);
            OH_AVFormat_GetIntValue(trackFormat.get(), OH_MD_KEY_DURATION, &info.durationTime);
            char *codecMime;
            OH_AVFormat_GetStringValue(trackFormat.get(), OH_MD_KEY_CODEC_MIME, const_cast<char const **>(&codecMime));
            info.codecMime = codecMime;
            OH_AVFormat_GetIntValue(trackFormat.get(), OH_MD_KEY_PROFILE, &info.hevcProfile);
            videoTrackId_ = index;

        } else if (trackType == MEDIA_TYPE_AUD) {
            OH_AVDemuxer_SelectTrackByID(demuxer, index);
            OH_AVFormat_GetLongValue(trackFormat.get(), OH_MD_KEY_BITRATE, &info.audioBitrate);
            OH_AVFormat_GetIntValue(trackFormat.get(),
                OH_MD_KEY_AUD_SAMPLE_RATE, reinterpret_cast<int32_t *>(&info.sampleRate));
            OH_AVFormat_GetIntValue(trackFormat.get(),
                OH_MD_KEY_AUD_CHANNEL_COUNT, reinterpret_cast<int32_t *>(&info.channelCount));

            char *audioCodecMime;
            OH_AVFormat_GetStringValue(trackFormat.get(),
                OH_MD_KEY_CODEC_MIME, const_cast<char const **>(&audioCodecMime));
            info.audioCodecMime = audioCodecMime;
            audioTrackId_ = index;
        }
    }
    return AV_ERR_OK;
}

int32_t Demuxer::ReadSample(OH_AVBuffer *buffer, OH_AVCodecBufferAttr &attr)
{
    int32_t ret = OH_AVDemuxer_ReadSampleBuffer(demuxer, videoTrackId_, buffer);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) << "read sample failed";
        return AV_ERR_UNKNOWN;
    }
    
    ret = OH_AVBuffer_GetBufferAttr(buffer, &attr);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) << "get buffer attr failed";
        return AV_ERR_UNKNOWN;
    }
    return AV_ERR_OK;
}

int32_t Demuxer::ReadAudioSample(OH_AVBuffer *buffer, OH_AVCodecBufferAttr &attr)
{
    int32_t ret = OH_AVDemuxer_ReadSampleBuffer(demuxer, audioTrackId_, buffer);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) << "read audio sample failed";
        return AV_ERR_UNKNOWN;
    }

    ret = OH_AVBuffer_GetBufferAttr(buffer, &attr);
    if (ret != AV_ERR_OK) {
        DLOG(ERROR) << "get audio buffer attr failed";
        return AV_ERR_UNKNOWN;
    }
    return AV_ERR_OK;
}

int32_t Demuxer::Release()
{
    if (source != nullptr) {
        OH_AVSource_Destroy(source);
        source = nullptr;
    }
    if (demuxer != nullptr) {
        OH_AVDemuxer_Destroy(demuxer);
        demuxer = nullptr;
    }
    return AV_ERR_OK;
}

} // namespace RNSkia
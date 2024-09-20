//
// Created on 2024/5/20.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef HARMONY_SVGVIEWMANAGER_H
#define HARMONY_SVGVIEWMANAGER_H

#include "HarmonyPlatformContext.h"
#include <memory>
#include <rawfile/raw_file_manager.h>

namespace RNSkia {

class SkiaManager {
public:
    static SkiaManager &getInstance() {
        static SkiaManager instance;
        return instance;
    }

    void setContext(std::shared_ptr<RNSkia::HarmonyPlatformContext> context);

    std::shared_ptr<RNSkia::HarmonyPlatformContext> getContext();

    void setManager(std::shared_ptr<RNSkia::RNSkManager> manager);
    std::shared_ptr<RNSkia::RNSkManager> getManager();

    void setReleaseVideo(bool relv);
    bool getReleaseVideo();
    
    static void *_pixels;
    struct Options {
        int width;
        int height;
        int stride;
        int pixelFormat;
        int alphaType;
    };
    static Options option ;
    static napi_value TagGetView(napi_env env, napi_callback_info info) {

        size_t argc = 6;
        napi_value args[6] = {nullptr};
        napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
        int32_t width;
        napi_get_value_int32(env, args[0], &width);

        int32_t height;
        napi_get_value_int32(env, args[1], &height);
        int32_t stride;
        napi_get_value_int32(env, args[2], &stride);

        int32_t pixelFormat;
        napi_get_value_int32(env, args[3], &pixelFormat);
        int32_t alphaType;
        napi_get_value_int32(env, args[4], &alphaType);
        option.height = height;
        option.width = width;
        option.pixelFormat = pixelFormat;
        option.alphaType = alphaType;
        option.stride = stride;

        OHOS::Media::OH_AccessPixels(env, args[5], &_pixels);

        DLOG(INFO) << "napi ok ,_pixels ok";
        return nullptr;
    }

private:
    SkiaManager() {}

    SkiaManager(const SkiaManager &) = delete;
    SkiaManager &operator=(const SkiaManager &) = delete;

    std::shared_ptr<RNSkia::HarmonyPlatformContext> platformContext;
    std::shared_ptr<RNSkia::RNSkManager> rnSkManager;
    bool releaseVideo;
};

} // namespace RNSkia

#endif // HARMONY_SVGVIEWMANAGER_H

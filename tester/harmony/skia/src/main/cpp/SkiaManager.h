//
// Created on 2024/5/20.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef HARMONY_SVGVIEWMANAGER_H
#define HARMONY_SVGVIEWMANAGER_H

#include "HarmonyPlatformContext.h"
#include <memory>

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

private:
    SkiaManager() {}

    SkiaManager(const SkiaManager &) = delete;
    SkiaManager &operator=(const SkiaManager &) = delete;

    std::shared_ptr<RNSkia::HarmonyPlatformContext> platformContext;
    std::shared_ptr<RNSkia::RNSkManager> rnSkManager;
};

} // namespace RNSkia

#endif // HARMONY_SVGVIEWMANAGER_H

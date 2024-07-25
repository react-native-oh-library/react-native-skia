//
// Created on 2024/5/20.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "SkiaManager.h"
#include <utility>

namespace RNSkia {

void SkiaManager::setContext(std::shared_ptr<RNSkia::HarmonyPlatformContext> context) { platformContext = context; };

std::shared_ptr<RNSkia::HarmonyPlatformContext> SkiaManager::getContext() { return platformContext; }

void SkiaManager::setManager(std::shared_ptr<RNSkia::RNSkManager> manager) { rnSkManager = manager; };

std::shared_ptr<RNSkia::RNSkManager> SkiaManager::getManager() { return rnSkManager; };


} // namespace RNSkia
#include "RNSkiaModule.h"
#include "RNOH/ArkTSTurboModule.h"
#include "SkiaManager.h"
#include <fstream>

using namespace facebook;
using namespace react;

namespace rnoh {

bool RNSkiaModule::install(jsi::Runtime &rt) {
    DLOG(INFO) << "RNSkiaModule::install";
    auto pixelDensity = 300;

    platformContext = std::make_shared<RNSkia::HarmonyPlatformContext>(&rt, jsInvoker_, pixelDensity);
    RNSkia::SkiaManager::getInstance().setContext(platformContext);
    rNSkManager = std::make_shared<RNSkia::RNSkManager>(&rt, jsInvoker_, platformContext);
    RNSkia::SkiaManager::getInstance().setManager(rNSkManager);

    DLOG(INFO) << "RNSkiaModule::install finish.";
    return true;
}

static jsi::Value __hostFunction_RNSkiaInstallModule_install(jsi::Runtime &rt, react::TurboModule &turboModule,
                                                             const jsi::Value *args, size_t count) {
    auto self = static_cast<RNSkiaModule *>(&turboModule);
    self->install(rt);
    return true;
};

RNSkiaModule::RNSkiaModule(const ArkTSTurboModule::Context ctx, const std::string name) : ArkTSTurboModule(ctx, name) {
    methodMap_["install"] = MethodMetadata{0, __hostFunction_RNSkiaInstallModule_install};
}
} // namespace rnoh

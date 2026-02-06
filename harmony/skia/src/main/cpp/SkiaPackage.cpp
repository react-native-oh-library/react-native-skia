/*
 * Copyright (c) 2024 Huawei Device Co., Ltd. All rights reserved
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

#include "SkiaPackage.h"
#include "RNSkiaModule.h"
#include "SkiaViewJSIBinder.h"
#include "SkiaPictureViewComponentDescriptor.h"


namespace rnoh {

class SkiaTurboModuleFactoryDelegate : public TurboModuleFactoryDelegate {
public:
    SharedTurboModule createTurboModule(Context ctx, const std::string &name) const override {
        if (name == "RNSkiaModule") {
            return std::make_shared<RNSkiaModule>(ctx, name);
        }
        return nullptr;
    };
};

std::unique_ptr<TurboModuleFactoryDelegate> SkiaPackage::createTurboModuleFactoryDelegate() {
    return std::make_unique<SkiaTurboModuleFactoryDelegate>();
}

std::vector<facebook::react::ComponentDescriptorProvider> SkiaPackage::createComponentDescriptorProviders() {
    return {
         facebook::react::concreteComponentDescriptorProvider<facebook::react::SkiaPictureViewComponentDescriptor>(),
    };
}

ComponentJSIBinderByString SkiaPackage::createComponentJSIBinderByName() {
    return {{"SkiaPictureView", std::make_shared<SkiaPictureViewJSIBinder>()}
    };
};
} // namespace rnoh
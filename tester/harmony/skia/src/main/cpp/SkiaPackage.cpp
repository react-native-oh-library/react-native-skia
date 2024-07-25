#include "SkiaPackage.h"
#include "RNSkiaModule.h"
#include "SkiaDomViewNapiBinder.h"
#include "SkiaViewJSIBinder.h"


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
        facebook::react::concreteComponentDescriptorProvider<facebook::react::SkiaDomViewComponentDescriptor>(),
    };
}

ComponentJSIBinderByString SkiaPackage::createComponentJSIBinderByName() {
    return {{"SkiaDomView", std::make_shared<SkiaDomViewJSIBinder>()}};
};

ComponentNapiBinderByString SkiaPackage::createComponentNapiBinderByName() {
    return {{"SkiaDomView", std::make_shared<SkiaDomViewNapiBinder>()}};
};

} // namespace rnoh
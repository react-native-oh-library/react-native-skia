#include "RNOH/Package.h"

namespace rnoh {
class SkiaPackage : public Package {

public:
    explicit SkiaPackage(Package::Context ctx) : Package(ctx) {}

    std::unique_ptr<TurboModuleFactoryDelegate> createTurboModuleFactoryDelegate() override;
    std::vector<facebook::react::ComponentDescriptorProvider> createComponentDescriptorProviders() override;
    ComponentJSIBinderByString createComponentJSIBinderByName() override;
    ComponentNapiBinderByString createComponentNapiBinderByName() override;
};

} // namespace rnoh

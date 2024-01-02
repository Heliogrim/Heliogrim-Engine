#include "DirectionalLightActor.hpp"

#include "../ActorInitializer.hpp"

using namespace hg;

DirectionalLightActor::DirectionalLightActor(cref<ActorInitializer> initializer_) :
    InheritMeta(initializer_),
    _directionalLightComponent(nullptr) {
    _directionalLightComponent = initializer_.createComponent<DirectionalLightComponent>(this);
}

ptr<DirectionalLightComponent> DirectionalLightActor::getDirectionalLightComponent() const noexcept {
    return _directionalLightComponent;
}

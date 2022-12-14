#include "CameraActor.hpp"

#include "../ActorInitializer.hpp"

using namespace ember;

CameraActor::CameraActor(cref<ActorInitializer> initializer_) :
    Actor(initializer_),
    _cameraComponent(nullptr),
    _mountComponent(nullptr) {

    _mountComponent = initializer_.createComponent<ActorComponent>(this);
    _cameraComponent = initializer_.createSubComponent<CameraComponent>(this, _mountComponent);
}

const ptr<CameraComponent> CameraActor::getCameraComponent() const noexcept {
    return _cameraComponent;
}

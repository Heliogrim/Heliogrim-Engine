#include "CameraActor.hpp"

#include "../ActorInitializer.hpp"

using namespace ember;

CameraActor::CameraActor() :
    Actor(),
    _cameraComponent(nullptr),
    _mountComponent(nullptr) {

    auto& init { ActorInitializer::get() };

    _mountComponent = init.createComponent<ActorComponent>(this);
    _cameraComponent = init.createSubComponent<CameraComponent>(this, _mountComponent);
}

const ptr<CameraComponent> CameraActor::getCameraComponent() const noexcept {
    return _cameraComponent;
}

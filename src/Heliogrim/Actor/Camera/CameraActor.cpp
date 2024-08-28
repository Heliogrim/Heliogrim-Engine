#include "CameraActor.hpp"

#include "../../ActorInitializer.hpp"

using namespace hg;

CameraActor::CameraActor(cref<ActorInitializer> initializer_) :
	InheritMeta(initializer_),
	_cameraComponent(nullptr),
	_mountComponent(nullptr) {

	_mountComponent = initializer_.createComponent<HierarchyComponent>(this);
	_cameraComponent = initializer_.createSubComponent<CameraComponent>(this, _mountComponent);
}

nmpt<CameraComponent> CameraActor::getCameraComponent() const noexcept {
	return _cameraComponent;
}

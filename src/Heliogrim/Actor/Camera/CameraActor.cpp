#include "CameraActor.hpp"

#include "../../ActorInitializer.hpp"

using namespace hg;

CameraActor::CameraActor(cref<ActorInitializer> initializer_) :
	InheritMeta(initializer_),
	_cameraComponent(nullptr),
	_mountComponent(nullptr) {

	if (initializer_.shouldAutoInitialize()) {
		_mountComponent = initializer_.createComponent<HierarchyComponent>(*this, { .parent = None });
		_cameraComponent = initializer_.createComponent<CameraComponent>(*this, { .parent = Some(*_mountComponent) });
	}
}

nmpt<CameraComponent> CameraActor::getCameraComponent() const noexcept {
	return _cameraComponent;
}

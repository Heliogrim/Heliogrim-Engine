#include "DirectionalLightActor.hpp"

#include "../../ActorInitializer.hpp"

using namespace hg;

DirectionalLightActor::DirectionalLightActor(cref<ActorInitializer> initializer_) :
	InheritMeta(initializer_),
	_directionalLightComponent(nullptr) {

	if (initializer_.shouldAutoInitialize()) {
		_directionalLightComponent = initializer_.createComponent<DirectionalLightComponent>(*this, { .parent = None });
	}
}

nmpt<DirectionalLightComponent> DirectionalLightActor::getDirectionalLightComponent() const noexcept {
	return _directionalLightComponent;
}

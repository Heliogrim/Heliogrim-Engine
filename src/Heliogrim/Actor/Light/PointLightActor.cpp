#include "PointLightActor.hpp"

#include "../../ActorInitializer.hpp"

using namespace hg;

PointLightActor::PointLightActor(cref<ActorInitializer> initializer_) :
	InheritMeta(initializer_),
	_pointLightComponent(nullptr) {
	_pointLightComponent = initializer_.createComponent<PointLightComponent>(*this, { .parent = None });
}

nmpt<PointLightComponent> PointLightActor::getPointLightComponent() const noexcept {
	return _pointLightComponent;
}
#include "StaticGeometryActor.hpp"

#include "../../ActorInitializer.hpp"

using namespace hg;

StaticGeometryActor::StaticGeometryActor(cref<ActorInitializer> initializer_) :
	InheritMeta(initializer_),
	_staticGeometryComponent(nullptr) {

	if (initializer_.shouldAutoInitialize()) {
		_staticGeometryComponent = initializer_.createComponent<StaticGeometryComponent>(*this, { .parent = None });
	}
}

nmpt<StaticGeometryComponent> StaticGeometryActor::getStaticGeometryComponent() const noexcept {
	return _staticGeometryComponent;
}

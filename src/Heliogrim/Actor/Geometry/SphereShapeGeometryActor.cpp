#include "SphereShapeGeometryActor.hpp"

using namespace hg;

SphereShapeGeometryActor::SphereShapeGeometryActor(cref<ActorInitializer> initializer_) :
	InheritMeta(initializer_),
	_sphereShapeCollisionComponent(nullptr),
	_sphereShapeGeometryComponent(nullptr) {

	if (initializer_.shouldAutoInitialize()) {
		_sphereShapeCollisionComponent = initializer_.createComponent<SphereShapeCollisionComponent>(*this, { .parent = None });;
		_sphereShapeGeometryComponent = initializer_.createComponent<SphereShapeGeometryComponent>(
			*this,
			{ .parent = Some<ref<HierarchyComponent>>(*_sphereShapeCollisionComponent) }
		);
	}
}

nmpt<SphereShapeCollisionComponent> SphereShapeGeometryActor::getSphereShapeCollisionComponent() const noexcept {
	return _sphereShapeCollisionComponent;
}

nmpt<SphereShapeGeometryComponent> SphereShapeGeometryActor::getSphereShapeGeometryComponent() const noexcept {
	return _sphereShapeGeometryComponent;
}

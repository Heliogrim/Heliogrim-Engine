#include "BoxShapeGeometryActor.hpp"

using namespace hg;

BoxShapeGeometryActor::BoxShapeGeometryActor(cref<ActorInitializer> initializer_) :
	InheritMeta(initializer_),
	_boxShapeCollisionComponent(nullptr),
	_boxShapeGeometryComponent(nullptr) {

	if (initializer_.shouldAutoInitialize()) {
		_boxShapeCollisionComponent = initializer_.createComponent<BoxShapeCollisionComponent>(*this, { .parent = None });
		_boxShapeGeometryComponent = initializer_.createComponent<BoxShapeGeometryComponent>(
			*this,
			{ .parent = Some<ref<HierarchyComponent>>(*_boxShapeCollisionComponent) }
		);
	}
}

nmpt<BoxShapeCollisionComponent> BoxShapeGeometryActor::getBoxShapeCollisionComponent() const noexcept {
	return _boxShapeCollisionComponent;
}

nmpt<BoxShapeGeometryComponent> BoxShapeGeometryActor::getBoxShapeGeometryComponent() const noexcept {
	return _boxShapeGeometryComponent;
}

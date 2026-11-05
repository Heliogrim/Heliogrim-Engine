#include "BoxShapeGeometryActor.hpp"

#include "../Archetype/ArchetypeDefinition.inl.hpp"
#include "../../Component/Scene/Collision/BoxShapeCollisionComponent.Layout.hpp"
#include "../../Component/Scene/Geometry/BoxShapeGeometryComponent.Layout.hpp"

using namespace hg;

BoxShapeGeometryActor::BoxShapeGeometryActor(cref<ActorInitializer> initializer_) :
	InheritArchetype(initializer_) {}

nmpt<BoxShapeCollisionComponent> BoxShapeGeometryActor::getBoxShapeCollisionComponent() noexcept {
	return &get<"collision">();
}

nmpt<BoxShapeGeometryComponent> BoxShapeGeometryActor::getBoxShapeGeometryComponent() noexcept {
	return &get<"geometry">();
}

template class hg::InheritArchetype<BoxShapeGeometryActor, Actor>;

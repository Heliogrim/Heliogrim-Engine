#include "BoxShapeGeometryActor.hpp"

#include <Engine.ACS/Archetype/ArchetypeDefinition.inl.hpp>

#include <Engine.Physics.Components/Collision/BoxShapeCollisionComponent.Layout.hpp>
#include <Engine.Graphics.Components/Geometry/BoxShapeGeometryComponent.Layout.hpp>

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

#include "SphereShapeGeometryActor.hpp"

#include <Engine.ACS/Archetype/ArchetypeDefinition.inl.hpp>
#include <Engine.Graphics.Components/Geometry/SphereShapeGeometryComponent.Layout.hpp>

import Engine.ACS;
import Engine.Physics.Components;

using namespace hg;

SphereShapeGeometryActor::SphereShapeGeometryActor(cref<ActorInitializer> initializer_) :
	InheritArchetype(initializer_) {}

nmpt<SphereShapeCollisionComponent> SphereShapeGeometryActor::getSphereShapeCollisionComponent() noexcept {
	return &get<"collision">();
}

nmpt<SphereShapeGeometryComponent> SphereShapeGeometryActor::getSphereShapeGeometryComponent() noexcept {
	return &get<"geometry">();
}

template class hg::InheritArchetype<SphereShapeGeometryActor, Actor>;

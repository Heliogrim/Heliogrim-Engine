#include "SphereShapeGeometryActor.hpp"

#include "../../Component/Scene/Collision/SphereShapeCollisionComponent.Layout.hpp"
#include "../../Component/Scene/Geometry/SphereShapeGeometryComponent.Layout.hpp"
#include "../Archetype/ArchetypeDefinition.inl.hpp"

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

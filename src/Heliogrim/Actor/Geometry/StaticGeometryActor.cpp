#include "StaticGeometryActor.hpp"

#include "../Archetype/ArchetypeDefinition.inl.hpp"
#include "../../Component/Scene/Geometry/StaticGeometryComponent.Layout.hpp"

using namespace hg;

StaticGeometryActor::StaticGeometryActor(cref<ActorInitializer> initializer_) :
	InheritArchetype(initializer_) {}

nmpt<StaticGeometryComponent> StaticGeometryActor::getStaticGeometryComponent() noexcept {
	return &get<"geometry">();
}

template class hg::InheritArchetype<StaticGeometryActor, Actor>;

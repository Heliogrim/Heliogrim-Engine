#include "StaticGeometryActor.hpp"

#include <Engine.ACS/Archetype/ArchetypeDefinition.inl.hpp>
#include <Engine.Graphics.Components/Geometry/StaticGeometryComponent.Layout.hpp>

import Engine.ACS;

using namespace hg;

StaticGeometryActor::StaticGeometryActor(cref<ActorInitializer> initializer_) :
	InheritArchetype(initializer_) {}

nmpt<StaticGeometryComponent> StaticGeometryActor::getStaticGeometryComponent() noexcept {
	return &get<"geometry">();
}

template class hg::InheritArchetype<StaticGeometryActor, Actor>;

#include "StaticGeometryActor.hpp"

#include <Engine.ACS/Archetype/ArchetypeDefinition.inl.hpp>

import Engine.ACS;
import Engine.Graphics.Components;

using namespace hg;

StaticGeometryActor::StaticGeometryActor(cref<ActorInitializer> initializer_) :
	InheritArchetype(initializer_) {}

nmpt<StaticGeometryComponent> StaticGeometryActor::getStaticGeometryComponent() noexcept {
	return &get<"geometry">();
}

template class hg::InheritArchetype<StaticGeometryActor, Actor>;

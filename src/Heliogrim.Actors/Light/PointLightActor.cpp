#include "PointLightActor.hpp"

#include <Engine.ACS/Archetype/ArchetypeDefinition.inl.hpp>

import Engine.ACS;
import Engine.Graphics.Components;

using namespace hg;

PointLightActor::PointLightActor(cref<ActorInitializer> initializer_) :
	InheritArchetype(initializer_) {}

nmpt<PointLightComponent> PointLightActor::getPointLightComponent() noexcept {
	return &get<"point">();
}

template class hg::InheritArchetype<PointLightActor, Actor>;

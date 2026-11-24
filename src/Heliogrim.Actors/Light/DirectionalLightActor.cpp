#include "DirectionalLightActor.hpp"

#include <Engine.ACS/Archetype/ArchetypeDefinition.inl.hpp>

import Engine.ACS;
import Engine.Graphics.Components;

using namespace hg;

DirectionalLightActor::DirectionalLightActor(cref<ActorInitializer> initializer_) :
	InheritArchetype(initializer_) {}

nmpt<DirectionalLightComponent> DirectionalLightActor::getDirectionalLightComponent() noexcept {
	return &get<"directional">();
}

template class hg::InheritArchetype<DirectionalLightActor, Actor>;

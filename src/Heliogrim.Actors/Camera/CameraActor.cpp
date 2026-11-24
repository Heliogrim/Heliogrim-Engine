#include "CameraActor.hpp"

#include <Engine.ACS/Archetype/ArchetypeDefinition.inl.hpp>

import Engine.ACS;
import Engine.Graphics.Components;

using namespace hg;

CameraActor::CameraActor(cref<ActorInitializer> initializer_) :
	InheritArchetype(initializer_) {}

nmpt<CameraComponent> CameraActor::getCameraComponent() noexcept {
	return &get<"camera">();
}

template class hg::InheritArchetype<CameraActor, Actor>;

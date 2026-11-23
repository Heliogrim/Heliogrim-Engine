#include <Engine.Common/Sal.hpp>

import Editor.Boot;
import Engine.ACS;
import Heliogrim.Actors;

using namespace hg;

void editor::boot::initActorClasses(_In_ [[maybe_unused]] ref<engine::ActorModule> module_) {
	std::ignore = RegisterActorClass<StaticGeometryActor>();
	std::ignore = RegisterActorClass<BoxShapeGeometryActor>();
	std::ignore = RegisterActorClass<SphereShapeGeometryActor>();
}

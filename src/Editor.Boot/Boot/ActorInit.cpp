#include <Engine.ACS/Actor/Actor.hpp>
#include <Heliogrim.Actors/Geometry/BoxShapeGeometryActor.hpp>
#include <Heliogrim.Actors/Geometry/SphereShapeGeometryActor.hpp>
#include <Heliogrim.Actors/Geometry/StaticGeometryActor.hpp>

import Editor.Boot;

using namespace hg;

void editor::boot::initActorClasses(_In_ [[maybe_unused]] ref<engine::ActorModule> module_) {
	std::ignore = RegisterActorClass<StaticGeometryActor>();
	std::ignore = RegisterActorClass<BoxShapeGeometryActor>();
	std::ignore = RegisterActorClass<SphereShapeGeometryActor>();
}

#include "ActorInit.hpp"

#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Actor/Geometry/BoxShapeGeometryActor.hpp>
#include <Heliogrim/Actor/Geometry/SphereShapeGeometryActor.hpp>
#include <Heliogrim/Actor/Geometry/StaticGeometryActor.hpp>

using namespace hg;

void editor::boot::initActorClasses(_In_ [[maybe_unused]] ref<engine::ActorModule> module_) {
	std::ignore = RegisterActorClass<StaticGeometryActor>();
	std::ignore = RegisterActorClass<BoxShapeGeometryActor>();
	std::ignore = RegisterActorClass<SphereShapeGeometryActor>();
}

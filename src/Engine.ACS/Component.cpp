#include "Component.hpp"

#include <Engine.Common/Move.hpp>
#include <Engine.Core/Engine.hpp>
#include <Heliogrim/Actor/ActorGuid.hpp>
#include <Heliogrim/Component/LogicComponent.hpp>

#include "ActorModule.hpp"
#include "Registry.hpp"

using namespace hg;

template <>
void VolatileComponent<LogicComponent>::destroy(ptr<LogicComponent> obj_) {
	auto registry = engine::Engine::getEngine()->getActors()->getRegistry();
	registry->releaseActorComponent(VolatileComponent<> { ::hg::move(obj_) });
}
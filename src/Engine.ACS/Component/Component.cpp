#include "Component.hpp"

#include <Heliogrim/Actor/ActorGuid.hpp>
#include <Heliogrim/Component/LogicComponent.hpp>

#include "ActorModule.hpp"
#include "Registry.hpp"

import Engine.Common;
import Engine.Core;

using namespace hg;

template <>
void VolatileComponent<LogicComponent>::destroy(ptr<LogicComponent> obj_) {
	auto registry = engine::Engine::getEngine()->getActors()->getRegistry();
	registry->releaseActorComponent(VolatileComponent<> { ::hg::move(obj_) });
}

#include "Component.hpp"

#include <Engine.ACS/Actor/ActorGuid.hpp>
#include <Engine.ACS/Component/LogicComponent.hpp>

#include "../ActorModule.hpp"
#include "../Registry.hpp"

import Engine.Common;
import Engine.Core;

using namespace hg;

template <>
void VolatileComponent<LogicComponent>::destroy(ptr<LogicComponent> obj_) {
	auto registry = engine::Engine::getEngine()->getActors()->getRegistry();
	registry->releaseActorComponent(VolatileComponent<> { ::hg::move(obj_) });
}

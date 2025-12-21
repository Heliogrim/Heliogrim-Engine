#include "Housekeeping.hpp"

#include <Engine.Common/Move.hpp>
#include <Heliogrim/Component/LogicComponent.hpp>

#include "Registry.hpp"

using namespace hg::engine::acs;
using namespace hg;

Housekeeping::Housekeeping(ref<Registry> registry_) :
	_registry(registry_),
	_pendingReleases() {}

Housekeeping::~Housekeeping() {
	commit();
}

void Housekeeping::storePendingRelease(mref<VolatileComponent<>> component_) {
	const auto* const component = ::hg::move(component_).release();
	const auto meta = component->getMetaClass<>();

	const auto where = _pendingReleases.find(ComponentTypeId { meta->typeId() });
	if (where != _pendingReleases.end()) {
		where->second.emplace_back(component);
		return;
	}

	_pendingReleases.emplace(
		std::make_pair(
			ComponentTypeId { meta->typeId() },
			Vector<ptr<const LogicComponent>> { component }
		)
	);
}

void Housekeeping::commit() {
	for (auto&& [type, batch] : _pendingReleases) {
		for (auto&& component : batch) {
			_registry.removeActorComponent(component->getComponentGuid(), type);
		}
	}
	_pendingReleases.clear();
}
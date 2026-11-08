#include "Housekeeping.hpp"

#include <Engine.Asserts/Asserts.hpp>
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

	const concurrent::SpinLock lock { _lck };
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

bool Housekeeping::commit() {
	decltype(_pendingReleases) pending {};
	{
		const concurrent::SpinLock lock { _lck };
		std::swap(pending, _pendingReleases);
	}

	for (auto&& [type, batch] : pending) {
		for (auto&& component : batch) {
			_registry.removeActorComponent(component->guid(), type);
		}
	}

	{
		// A release during commit means teardown overlaps the housekeeping stage (stop ordering bug).
		const concurrent::SpinLock lock { _lck };
		::hg::assertd(_pendingReleases.empty());
	}
	return _registry.reclaimSlots();
}

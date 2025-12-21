#include "ScopedSlotGuard.hpp"

#ifdef _DEBUG
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Logging/Logger.hpp>
#include "StructureSlotTypeTraits.hpp"
#endif

#include "ScopedStructureSlot.hpp"

using namespace hg::engine::serialization;
using namespace hg;

ScopedSlotGuard::ScopedSlotGuard(
	const non_owning_rptr<const ScopedSlot> scopedSlot_,
	const ScopedSlotGuardMode mode_
) :
	_mode(mode_),
	_scopedSlot(scopedSlot_) {

	auto* const slot = _scopedSlot->slot();
	if (_mode == ScopedSlotGuardMode::eRead) {
		slot->readHeader();

		#ifdef _DEBUG
		if (not
			slot->validateType()) {
			IM_CORE_ERRORF(
				"Tried to deserialize a `{}` into a `{}`.",
				StructureSlotTypeTrait::canonical(slot->getSlotHeader().type),
				StructureSlotTypeTrait::canonical(slot->getSlotType())
			);
			::hg::assertd(slot->validateType());
		}
		#endif

	} else {
		slot->writeHeader();
	}

	slot->enter();
}

ScopedSlotGuard::~ScopedSlotGuard() {

	auto* const slot = _scopedSlot->slot();
	slot->leave();

	if (_mode == ScopedSlotGuardMode::eRead) {
		// __noop();

	} else {
		slot->writeHeader();
	}
}
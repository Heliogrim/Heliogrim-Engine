#include "StringScopedSlot.hpp"

#include "ScopedSlotGuard.hpp"
#include "StringSlot.hpp"

import Engine.Common;

using namespace hg::engine::serialization;
using namespace hg;

StringScopedSlot::StringScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_) :
	TypeScopedSlot(std::move(scopedState_), make_sptr<StringSlot>(std::move(state_))) {}

StringScopedSlot::~StringScopedSlot() = default;

void StringScopedSlot::operator<<(cref<value_type> value_) {
	const ScopedSlotGuard guard { this, ScopedSlotGuardMode::eWrite };
	(*slot()) << value_;
}

void StringScopedSlot::operator>>(ref<value_type> value_) const {
	const ScopedSlotGuard guard { this, ScopedSlotGuardMode::eRead };
	(*slot()) >> value_;
}

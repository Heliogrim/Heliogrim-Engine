#include "StringScopedSlot.hpp"

#include <span>
#include <Engine.Common/Make.hpp>

#include "../Archive/Archive.hpp"
#include "ScopedSlotGuard.hpp"
#include "StringSlot.hpp"

using namespace hg::engine::serialization;
using namespace hg;

StringScopedSlot::StringScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_) :
    TypeScopedSlot(_STD move(scopedState_), make_sptr<StringSlot>(_STD move(state_))) {}

StringScopedSlot::~StringScopedSlot() = default;

void StringScopedSlot::operator<<(cref<value_type> value_) {
    const ScopedSlotGuard guard { this, ScopedSlotGuardMode::eWrite };
    (*slot()) << value_;
}

void StringScopedSlot::operator>>(ref<value_type> value_) const {
    const ScopedSlotGuard guard { this, ScopedSlotGuardMode::eRead };
    (*slot()) >> value_;
}

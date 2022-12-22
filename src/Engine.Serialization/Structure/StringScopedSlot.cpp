#include "StringScopedSlot.hpp"

#include <span>

#include "../Archive/Archive.hpp"
#include "ScopedSlotGuard.hpp"
#include "StringSlot.hpp"

using namespace ember::engine::serialization;
using namespace ember;

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

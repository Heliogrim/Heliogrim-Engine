#include "ScopedStructureSlot.hpp"

using namespace ember::engine::serialization;
using namespace ember;

ScopedStructureSlotBase::ScopedStructureSlotBase(cref<ScopedSlotState> state_) :
    StructureSlotBase(),
    _state(state_) {}

ScopedStructureSlotBase::ScopedStructureSlotBase(mref<ScopedSlotState> state_) :
    StructureSlotBase(),
    _state(_STD move(state_)) {}

ScopedStructureSlotBase::~ScopedStructureSlotBase() {
    /* Report current slot to parent state */
    if (_state.isScopedDirty() && _STD addressof(_state.parent) != nullptr) {
        _state.parent.subStateLeave(*this);
    }
}

cref<ScopedSlotState> ScopedStructureSlotBase::getState() const noexcept {
    return _state;
}

bool ScopedStructureSlotBase::emptyState() const noexcept {
    return static_cast<bool>(_state.rootState);
}

void ScopedStructureSlotBase::subStateEnter(cref<ScopedStructureSlotBase> subState_) const {}

void ScopedStructureSlotBase::subStateLeave(cref<ScopedStructureSlotBase> subState_) const {}

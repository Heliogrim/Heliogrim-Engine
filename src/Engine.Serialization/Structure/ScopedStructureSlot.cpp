#include "ScopedStructureSlot.hpp"

#include "StructureSlotState.hpp"
#include "../Archive/Archive.hpp"

using namespace ember::engine::serialization;
using namespace ember;

ScopedStructureSlotBase::ScopedStructureSlotBase(cref<ScopedSlotState> state_) :
    StructureSlotBase(),
    _state(state_) {}

ScopedStructureSlotBase::ScopedStructureSlotBase(mref<ScopedSlotState> state_) :
    StructureSlotBase(),
    _state(_STD move(state_)) {}

ScopedStructureSlotBase::~ScopedStructureSlotBase() = default;

cref<ScopedSlotState> ScopedStructureSlotBase::getState() const noexcept {
    return _state;
}

bool ScopedStructureSlotBase::emptyState() const noexcept {
    return static_cast<bool>(_state.rootState);
}

void ScopedStructureSlotBase::ensureEntered(const bool mutating_) {

    if (mutating_ && _state.isScopedDirty()) {
        return;
    }

    enter(mutating_);

    if (mutating_ && _STD addressof(_state.parent) != nullptr) {
        const_cast<ref<ScopedStructureSlotBase>>(_state.parent).subStateEnter(*this);
    }
}

void ScopedStructureSlotBase::ensureLeft(const bool mutating_) {

    if (mutating_ && not _state.isScopedDirty()) {
        return;
    }

    leave(mutating_);

    if (mutating_ && _STD addressof(_state.parent) != nullptr) {
        const_cast<ref<ScopedStructureSlotBase>>(_state.parent).subStateLeave(*this);
    }
}

void ScopedStructureSlotBase::enter(const bool mutating_) {

    if (_state.offset != -1i64) {
        reenter(mutating_);
    } else {
        _state.offset = _state.rootState->getArchive()->tell();
    }

    if (mutating_) {
        _state.flags |= ScopedSlotStateFlag::eDirty;
    }
}

void ScopedStructureSlotBase::leave(const bool mutating_) {

    if (not mutating_) {
        return;
    }

    if (_state.size != 0i64) {
        // reentered slot or repeated leave propagation
        _state.size = _STD max(_state.size, _STD abs(_state.rootState->getArchive()->tell() - _state.offset));
    } else {
        _state.size = _STD abs(_state.rootState->getArchive()->tell() - _state.offset);
    }

    _state.flags.unwrap &= ~(static_cast<ScopedSlotStateFlags::value_type>(ScopedSlotStateFlag::eDirty));
}

void ScopedStructureSlotBase::reenter(const bool mutating_) {}

void ScopedStructureSlotBase::subStateEnter(cref<ScopedStructureSlotBase> subState_) {}

void ScopedStructureSlotBase::subStateLeave(cref<ScopedStructureSlotBase> subState_) {}

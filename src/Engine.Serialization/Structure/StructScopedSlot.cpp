#include "StructScopedSlot.hpp"

#include "StructIdentifierScopedSlot.hpp"

#ifdef _DEBUG
#include <Engine.Logging/Logger.hpp>
#endif

using namespace ember::engine::serialization;
using namespace ember;

StructScopedSlot::StructScopedSlot(cref<ScopedSlotState> state_) :
    ScopedStructureSlotBase(state_) {}

StructScopedSlot::StructScopedSlot(mref<ScopedSlotState> state_) :
    ScopedStructureSlotBase(_STD move(state_)) {}

RecordScopedSlot StructScopedSlot::insertRecordSlot(cref<record_key_type> key_) {

    ScopedSlotState identifierState {
        ScopedSlotStateFlag::eDirty,
        *this,
        _state.rootState
    };
    (StructIdentifierScopedSlot { _STD move(identifierState) }) << key_;

    /**/

    ScopedSlotState state {
        ScopedSlotStateFlag::eDirty,
        *this,
        _state.rootState
    };
    return RecordScopedSlot { _STD move(state) };
}

const RecordScopedSlot StructScopedSlot::getRecordSlot(cref<record_key_type> key_) const {

    ScopedSlotState identifierState {
        ScopedSlotStateFlag::eImmutable,
        *this,
        _state.rootState
    };

    string identifier {};
    (StructIdentifierScopedSlot { _STD move(identifierState) }) >> identifier;

    #ifdef _DEBUG
    if (identifier != key_) {
        IM_CORE_WARNF(
            "Tried to access serialized record `{}` with identifier key `{}`.",
            identifier,
            key_
        );
    }
    #endif

    /**/

    ScopedSlotState state {
        ScopedSlotStateFlag::eImmutable,
        *this,
        _state.rootState
    };
    return RecordScopedSlot { _STD move(state) };
}

const RecordScopedSlot StructScopedSlot::getRecordSlot(const size_t index_) const {
    throw NotImplementedException();
    /*
    ScopedSlotState state {
        ScopedSlotStateFlag::eImmutable,
        *this,
        _state.rootState
    };
    return RecordScopedSlot { _STD move(state) };
     */
}

const RecordScopedSlot StructScopedSlot::operator[](cref<record_key_type> key_) const {
    return getRecordSlot(key_);
}

#include "StructScopedSlot.hpp"

#include "StructIdentifierScopedSlot.hpp"

#ifdef _DEBUG
#include <Engine.Logging/Logger.hpp>
#include "StructureSlotTypeTraits.hpp"
#endif

using namespace ember::engine::serialization;
using namespace ember;

StructScopedSlot::StructScopedSlot(cref<ScopedSlotState> state_) :
    ScopedStructureSlotBase(state_) {}

StructScopedSlot::StructScopedSlot(mref<ScopedSlotState> state_) :
    ScopedStructureSlotBase(_STD move(state_)) {}

void StructScopedSlot::enter(const bool mutating_) {

    ScopedStructureSlotBase::enter(mutating_);

    /**/

    if (not mutating_ && not _state.size) {

        auto* archive = _state.rootState->getArchive();

        StructureSlotType storedType = StructureSlotType::eUndefined;
        (*archive) >> storedType;

        #ifdef _DEBUG
        if (storedType != StructureSlotType::eStruct) {
            IM_CORE_WARNF(
                "Tried to deserialize a `{}` into a `{}`",
                StructureSlotTypeTrait::canonical(storedType),
                StructureSlotTypeTraits<StructureSlotType::eStruct>::canonical
            );
        }
        #endif

        (*archive) >> _state.size;
        return;
    }

    /**/

    if (mutating_ && not _state.size) {
        auto* archive = _state.rootState->getArchive();
        (*archive) << StructureSlotType::eStruct;
        (*archive) << 0ui64;
    }
}

void StructScopedSlot::leave(const bool mutating_) {

    ScopedStructureSlotBase::leave(mutating_);

    /**/

    if (not mutating_) {
        return;
    }

    /**/

    auto* archive = _state.rootState->getArchive();
    archive->seek(_state.offset);

    (*archive) << StructureSlotType::eStruct;
    (*archive) << (_state.size - sizeof(StructureSlotType::eStruct) - sizeof(u64));

    archive->seek(_state.offset + _state.size);
}

void StructScopedSlot::subStateEnter(cref<ScopedStructureSlotBase> subState_) {
    ScopedStructureSlotBase::subStateEnter(subState_);
}

void StructScopedSlot::subStateLeave(cref<ScopedStructureSlotBase> subState_) {
    ScopedStructureSlotBase::subStateLeave(subState_);

    /**/

    ensureLeft(true);
    ensureEntered(true);
}

RecordScopedSlot StructScopedSlot::insertRecordSlot(cref<record_key_type> key_) {

    ensureEntered(true);

    /**/

    ScopedSlotState identifierState {
        ScopedSlotStateFlag::eUndefined,
        *this,
        _state.rootState
    };
    (StructIdentifierScopedSlot { _STD move(identifierState) }) << key_;

    /**/

    ScopedSlotState state {
        ScopedSlotStateFlag::eUndefined,
        *this,
        _state.rootState
    };
    return RecordScopedSlot { _STD move(state) };
}

const RecordScopedSlot StructScopedSlot::getRecordSlot(cref<record_key_type> key_) const {

    const ScopedSlotGuard guard { this, false };

    /**/

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

#include "StructScopedSlot.hpp"

#include "StructureSlotTypeTraits.hpp"
#include "StructSlot.hpp"
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Exception/NotImplementedException.hpp>

#ifdef _DEBUG
#include <Engine.Logging/Logger.hpp>
#endif

using namespace ember::engine::serialization;
using namespace ember;

StructScopedSlot::StructScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_) :
    ScopedSlot(_STD move(scopedState_), make_sptr<StructSlot>(_STD move(state_))) {

    if (not slot()) {
        return;
    }

    if ((static_cast<cref<ScopedSlotStateFlags>>(_state.flags) & ScopedSlotStateFlag::eDirty)) {
        slot()->writeHeader();
    } else {
        slot()->readHeader();
    }

    slot()->enter();
}

StructScopedSlot::~StructScopedSlot() {

    if (not slot()) {
        return;
    }

    slot()->leave();

    if ((static_cast<cref<ScopedSlotStateFlags>>(_state.flags) & ScopedSlotStateFlag::eDirty)) {
        slot()->writeHeader();
    }
}

RecordScopedSlot StructScopedSlot::insertRecordSlot(cref<record_key_type> key_) {

    auto record = static_cast<ptr<StructSlot>>(slot())->insertRecord(key_);
    ScopedSlotState scopedState { ScopedSlotStateFlag::eDirty };

    return RecordScopedSlot { _STD move(scopedState), _STD move(record) };
}

const RecordScopedSlot StructScopedSlot::getRecordSlot(cref<record_key_type> key_) const {

    auto record = static_cast<ptr<StructSlot>>(slot())->getRecord(key_);
    ScopedSlotState scopedState { ScopedSlotStateFlag::eClean };

    return RecordScopedSlot { _STD move(scopedState), _STD move(record) };
}

const RecordScopedSlot StructScopedSlot::getRecordSlot(const size_t index_) const {
    throw NotImplementedException();
}

#if FALSE
StructScopedSlot::StructScopedSlot(cref<ScopedSlotState> state_) :
    ScopedStructureSlotBase(state_) {
    ensureEntered(not _state.isScopedImmutable());
}

StructScopedSlot::StructScopedSlot(mref<ScopedSlotState> state_) :
    ScopedStructureSlotBase(_STD move(state_)) {
    ensureEntered(not _state.isScopedImmutable());
}

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

s64 StructScopedSlot::findRecord(cref<record_key_type> key_) const {

    auto* archive = _state.rootState->getArchive();

    const s64 start = _state.offset + static_cast<s64>(sizeof(StructureSlotType::eStruct) + sizeof(u64));
    if (archive->tell() != start) {
        archive->seek(start);
    }

    /**/

    auto leftSize = _state.size;
    while (leftSize >= 0) {

        ScopedSlotState identifierState {
            ScopedSlotStateFlag::eImmutable,
            *this,
            _state.rootState
        };

        string identifier {};
        (StructIdentifierScopedSlot { _STD move(identifierState) }) >> identifier;

        /**/

        if (identifier == key_) {
            //return start + (_state.size - leftSize);
            return archive->tell();
        }

        /**/

        s64 skipSize = 0;
        {
            StructureSlotType storedType = StructureSlotType::eUndefined;
            (*archive) >> storedType;

            const auto constTypeSize = StructureSlotTypeTrait::const_size(storedType);
            if (constTypeSize >= 0) {
                skipSize = constTypeSize;

            } else {

                s64 storedSize = -1;
                (*archive) >> storedSize;

                skipSize = storedSize;
            }
        }

        /**/
        archive->seek(archive->tell() + skipSize);
        leftSize = _state.size - (archive->tell() - start);
    }

    /**/

    return -1;
}

const RecordScopedSlot StructScopedSlot::getRecordSlot(cref<record_key_type> key_) const {

    const ScopedSlotGuard guard { this, false };

    /**/

    const auto testTell = _state.rootState->getArchive()->tell();
    auto result = findRecord(key_);
    _state.rootState->getArchive()->seek(testTell);

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

#endif

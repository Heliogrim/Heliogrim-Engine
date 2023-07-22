#include "StructSlot.hpp"

#include <Engine.Common/Make.hpp>

#include "../Archive/Archive.hpp"
#include "StringSlot.hpp"
#include "RecordSlot.hpp"

using namespace hg::engine::serialization;
using namespace hg;

StructSlot::StructSlot(cref<StructureSlotState> state_) :
    StructureSlotBase(state_) {
    _state.header = StructureSlotHeader::from<StructureSlotType::eStruct>();
}

StructSlot::StructSlot(mref<StructureSlotState> state_) :
    StructureSlotBase(_STD move(state_)) {
    _state.header = StructureSlotHeader::from<StructureSlotType::eStruct>();
}

StructSlot::~StructSlot() {
    if (_state.flags & StructureSlotStateFlag::eDirty) {
        leave();
    }
}

StructureSlotType StructSlot::getSlotType() const noexcept {
    return StructureSlotType::eStruct;
}

bool StructSlot::validateType() const noexcept {
    return _state.header.type == StructureSlotType::eStruct;
}

void StructSlot::feedback(const non_owning_rptr<const StructureSlotBase> other_) {

    const auto& header = other_->getSlotHeader();
    s64 add = header.size + sizeof(header.type);

    if (not header.sizeInferred) {
        add += sizeof(header.size);
    }

    /**/

    _state.header.size = MAX(_state.header.size, 0) + add;
    writeHeader();
}

s64 StructSlot::findRecord(cref<record_key_type> key_) const {

    constexpr auto off = sizeof(StructureSlotHeader::type) + sizeof(StructureSlotHeader::size);

    s64 offset = _state.offset + off;
    _state.root->archive->seek(offset);

    /**/

    const auto end = _state.offset + _state.header.size;
    string tmp {};

    while (offset < end) {

        StructureSlotState identifierState {
            StructureSlotStateFlag::eImmutable,
            offset,
            StructureSlotHeader::from<StructureSlotType::eString>(),
            const_cast<ptr<this_type>>(this)->shared_from_this(),
            _state.root
        };

        StringSlot identifier { _STD move(identifierState) };
        identifier.readHeader();
        identifier.enter();

        identifier >> tmp;

        identifier.leave();

        /**/

        offset += identifier.getState().header.size;
        offset += sizeof(StructureSlotHeader::type);
        offset += sizeof(StructureSlotHeader::size);

        /**/

        if (key_ == tmp) {
            break;
        }

        /**/

        StructureSlotType valueType = StructureSlotType::eUndefined;
        (*_state.root->archive) >> valueType;

        auto valueHeader = StructureSlotHeader::from(valueType);
        if (not valueHeader.sizeInferred) {
            (*_state.root->archive) >> valueHeader.size;
        }

        /**/

        offset += valueHeader.size;
        offset += sizeof(StructureSlotHeader::type);

        if (not valueHeader.sizeInferred) {
            offset += sizeof(StructureSlotHeader::size);
        }
    }

    /**/

    return tmp == key_ ? offset : -1i64;
}

sptr<RecordSlot> StructSlot::insertRecord(cref<record_key_type> key_) {

    _state.flags |= StructureSlotStateFlag::eDirty;

    /**/

    s64 offset = _state.offset + _state.header.size +
        sizeof(_state.header.type) + sizeof(_state.header.size);

    /**/

    {
        StructureSlotState identifierState {
            StructureSlotStateFlags { StructureSlotStateFlag::eMutable } | StructureSlotStateFlag::eDirty,
            offset,
            StructureSlotHeader::from<StructureSlotType::eRecord>(),
            shared_from_this(),
            _state.root
        };

        StringSlot identifier { _STD move(identifierState) };
        identifier.writeHeader();
        identifier.enter();
        identifier << key_;
        identifier.leave();
        identifier.writeHeader();

        /**/

        offset += identifier.getState().header.size;
        offset += sizeof(StructureSlotHeader::type);
        offset += sizeof(StructureSlotHeader::size);
    }

    /**/

    StructureSlotState slotState {
        StructureSlotStateFlags { StructureSlotStateFlag::eMutable } | StructureSlotStateFlag::eDirty,
        offset,
        StructureSlotHeader::from<StructureSlotType::eRecord>(),
        shared_from_this(),
        _state.root
    };

    return make_sptr<RecordSlot>(_STD move(slotState));
}

bool StructSlot::hasRecord(cref<record_key_type> key_) {
    return findRecord(key_) != -1;
}

sptr<RecordSlot> StructSlot::getRecord(cref<record_key_type> key_) {

    const s64 offset = findRecord(key_);

    /**/

    if (offset == -1) {
        return nullptr;
    }

    /**/

    StructureSlotState slotState {
        StructureSlotStateFlag::eImmutable,
        offset,
        StructureSlotHeader::from<StructureSlotType::eRecord>(),
        shared_from_this(),
        _state.root
    };

    return make_sptr<RecordSlot>(_STD move(slotState));
}

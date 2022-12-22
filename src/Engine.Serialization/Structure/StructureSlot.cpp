#include "StructureSlot.hpp"

#include "../Archive/Archive.hpp"

using namespace ember::engine::serialization;
using namespace ember;

StructureSlotBase::StructureSlotBase(cref<StructureSlotState> state_) :
    _state(state_) {}

StructureSlotBase::StructureSlotBase(mref<StructureSlotState> state_) :
    _state(_STD move(state_)) {}

cref<StructureSlotHeader> StructureSlotBase::getSlotHeader() const noexcept {
    return _state.header;
}

ref<StructureSlotHeader> StructureSlotBase::getSlotHeader() noexcept {
    return _state.header;
}

StructureSlotType StructureSlotBase::getSlotType() const noexcept {
    return _state.header.type;
}

void StructureSlotBase::writeHeader() {

    auto* const archive = _state.root->archive;
    archive->seek(_state.offset);

    (*archive) << _state.header.type;

    if (not _state.header.sizeInferred) {
        (*archive) << _state.header.size;
    }
}

void StructureSlotBase::readHeader() {

    auto* const archive = _state.root->archive;
    archive->seek(_state.offset);

    (*archive) >> _state.header.type;
    _state.header = StructureSlotHeader::from(_state.header.type);

    if (not _state.header.sizeInferred) {
        (*archive) >> _state.header.size;
    }
}

void StructureSlotBase::enter() {

    auto* const archive = _state.root->archive;

    if (_state.header.sizeInferred) {
        constexpr s64 off = sizeof(_state.header.type);
        archive->seek(_state.offset + off);

    } else {
        constexpr s64 off = sizeof(_state.header.type) + sizeof(_state.header.size);
        archive->seek(_state.offset + off);
    }
}

void StructureSlotBase::leave() {

    const auto* const archive = _state.root->archive;

    if (_state.header.sizeInferred) {
        return;

    } else {
        constexpr s64 off = sizeof(_state.header.type) + sizeof(_state.header.size);
        const auto start = _state.offset + off;

        _state.header.size = archive->tell() - start;
    }
}

void StructureSlotBase::feedback(const non_owning_rptr<const StructureSlotBase> other_) {}

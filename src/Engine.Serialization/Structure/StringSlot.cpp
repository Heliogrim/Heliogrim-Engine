#include "StringSlot.hpp"

#include <span>

#include "../Archive/Archive.hpp"

using namespace hg::engine::serialization;
using namespace hg;

StringSlot::StringSlot(cref<StructureSlotState> state_) :
    TypedStructureSlotBase(state_) {
    _state.header = StructureSlotHeader::from<StructureSlotType::eString>();
}

StringSlot::StringSlot(mref<StructureSlotState> state_) :
    TypedStructureSlotBase(std::move(state_)) {
    _state.header = StructureSlotHeader::from<StructureSlotType::eString>();
}

StructureSlotType StringSlot::getSlotType() const noexcept {
    return StructureSlotType::eString;
}

bool StringSlot::validateType() const noexcept {
    return _state.header.type == StructureSlotType::eString;
}

void StringSlot::operator<<(cref<value_type> value_) {

    const auto src = std::span<u8, std::dynamic_extent>(
        reinterpret_cast<ptr<u8>>(const_cast<ref<value_type>>(value_).data()),
        value_.size() * sizeof(value_type::value_type)
    );

    auto* const archive = _state.root->archive;
    archive->serializeBytes(src.data(), src.size(), ArchiveStreamMode::eIn);

    /**/

    _state.header.size = src.size();
}

SlotOpResult StringSlot::operator>>(ref<value_type> value_) {

    value_.resize(_state.header.size / sizeof(value_type::value_type));

    const auto dst = std::span<u8, std::dynamic_extent>(
        reinterpret_cast<ptr<u8>>(value_.data()),
        _state.header.size
    );

    auto* const archive = _state.root->archive;
    archive->serializeBytes(dst.data(), dst.size(), ArchiveStreamMode::eOut);

    return SlotOpResult::eSuccess;
}

#include "StringSlot.hpp"

#include <Engine.Resource.Archive/Archive.hpp>
#include <Engine.Resource.Archive/ArchiveStreamMode.hpp>

import std;

using namespace hg::engine::serialization;
using namespace hg;

StringSlot::StringSlot(cref<StructureSlotState> state_) noexcept :
	TypedStructureSlotBase(state_) {
	_state.header = StructureSlotHeader::from<StructureSlotType::eString>();
}

StringSlot::StringSlot(mref<StructureSlotState> state_) noexcept :
	TypedStructureSlotBase(std::move(state_)) {
	_state.header = StructureSlotHeader::from<StructureSlotType::eString>();
}

StructureSlotType StringSlot::getSlotType() const noexcept {
	return StructureSlotType::eString;
}

bool StringSlot::validateType() const noexcept {
	return _state.header.type == StructureSlotType::eString;
}

void StringSlot::operator<<(cref<StringView> value_) {

	const auto src = std::span<u8, std::dynamic_extent>(
		reinterpret_cast<ptr<u8>>(const_cast<ptr<char>>(value_.data())),
		value_.size() * sizeof(value_type::value_type)
	);

	const auto archive = _state.root->archive;
	archive->serializeBytes(src.data(), src.size(), resource::ArchiveStreamMode::eStore);

	/**/

	_state.header.size = src.size();
}

void StringSlot::operator<<(cref<value_type> value_) {

	const auto src = std::span<u8, std::dynamic_extent>(
		reinterpret_cast<ptr<u8>>(const_cast<ref<value_type>>(value_).data()),
		value_.size() * sizeof(value_type::value_type)
	);

	const auto archive = _state.root->archive;
	archive->serializeBytes(src.data(), src.size(), resource::ArchiveStreamMode::eStore);

	/**/

	_state.header.size = src.size();
}

SlotOpResult StringSlot::operator>>(ref<value_type> value_) {

	if (not hasPayload()) {
		return SlotOpResult::eFailed;
	}

	if (_state.header.type != StructureSlotType::eString) {
		fail(ReadError::Kind::eTypeMismatch, _state.offset, slotSpan(_state.header), "slot is not a string");
		return SlotOpResult::eFailed;
	}

	const auto size = _state.header.size;
	if (not charge(static_cast<u64>(size))) {
		return SlotOpResult::eFailed;
	}

	value_type tmp(static_cast<size_t>(size) / sizeof(value_type::value_type), '\0');

	const auto archive = _state.root->archive;
	archive->seek(payloadBegin());
	archive->serializeBytes(reinterpret_cast<ptr<u8>>(tmp.data()), static_cast<u64>(size), resource::ArchiveStreamMode::eLoad);

	if (not checkArchive(_state.offset, slotSpan(_state.header))) {
		return SlotOpResult::eFailed;
	}

	value_ = std::move(tmp);
	return SlotOpResult::eSuccess;
}

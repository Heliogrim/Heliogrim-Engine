#include "StructSlot.hpp"

#include <Engine.Common/__macro.hpp>

#include "RecordSlot.hpp"
#include "StringSlot.hpp"
#include "../Load/LoadContext.hpp"
#include "../Symbols.hpp"

import Engine.Common;
import Engine.Resource.Archive;

using namespace hg::engine::serialization;
using namespace hg;

StructSlot::StructSlot(cref<StructureSlotState> state_) noexcept :
	StructureSlotBase(state_) {
	_state.header = StructureSlotHeader::from<StructureSlotType::eStruct>();
}

StructSlot::StructSlot(mref<StructureSlotState> state_) noexcept :
	StructureSlotBase(std::move(state_)) {
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

	if (not hasPayload()) {
		return -1LL;
	}

	if (_state.header.type != StructureSlotType::eStruct) {
		fail(ReadError::Kind::eTypeMismatch, _state.offset, slotSpan(_state.header), "slot is not a struct");
		return -1LL;
	}

	/**/

	const auto archive = _state.root->archive;
	const s64 end = payloadEnd();

	s64 offset = payloadBegin();
	s64 found = -1LL;

	// shortcut: full scan per lookup so duplicate keys are always detected; index the keys if lookups get hot.
	while (offset < end) {

		StructureSlotHeader keyHeader {};
		if (not readHeaderAt(offset, end, keyHeader)) {
			return -1LL;
		}

		if (keyHeader.type != StructureSlotType::eString) {
			fail(ReadError::Kind::eMalformed, offset, slotSpan(keyHeader), "struct key is not a string");
			return -1LL;
		}

		const s64 keyBegin = offset + slotSpan(keyHeader) - keyHeader.size;
		const s64 valueAt = offset + slotSpan(keyHeader);

		bool match = false;
		if (static_cast<u64>(keyHeader.size) == key_.size()) {
			string tmp(static_cast<size_t>(keyHeader.size), '\0');
			archive->seek(keyBegin);
			archive->serializeBytes(reinterpret_cast<ptr<u8>>(tmp.data()), tmp.size(), resource::ArchiveStreamMode::eLoad);
			if (not checkArchive(offset, slotSpan(keyHeader))) {
				return -1LL;
			}
			match = tmp == key_;
		}

		StructureSlotHeader valueHeader {};
		if (not readHeaderAt(valueAt, end, valueHeader)) {
			return -1LL;
		}

		if (match) {
			if (found != -1LL) {
				fail(ReadError::Kind::eDuplicateKey, offset, slotSpan(keyHeader), "duplicate struct key");
				return -1LL;
			}
			found = valueAt;
		}

		offset = valueAt + slotSpan(valueHeader);
	}

	if (found != -1LL && _state.root->context) {
		_state.root->context->markRead(&*archive, _state.offset, key_);
	}
	return found;
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

		StringSlot identifier { std::move(identifierState) };
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

	return make_sptr<RecordSlot>(std::move(slotState));
}

bool StructSlot::hasRecord(cref<record_key_type> key_) {
	return findRecord(key_) != -1;
}

sptr<RecordSlot> StructSlot::getRecord(cref<record_key_type> key_) {

	/* A missing key yields an empty record (offset -1): reads through it fail and leave the target untouched. */
	StructureSlotState slotState {
		StructureSlotStateFlag::eImmutable,
		findRecord(key_),
		StructureSlotHeader::from<StructureSlotType::eRecord>(),
		shared_from_this(),
		_state.root
	};

	return make_sptr<RecordSlot>(std::move(slotState));
}

Vector<String> StructSlot::keys() const {

	Vector<String> out {};
	if (not hasPayload() || _state.header.type != StructureSlotType::eStruct) {
		return out;
	}

	const auto archive = _state.root->archive;
	const s64 end = payloadEnd();

	for (s64 offset = payloadBegin(); offset < end;) {

		StructureSlotHeader keyHeader {};
		if (not readHeaderAt(offset, end, keyHeader) || keyHeader.type != StructureSlotType::eString) {
			return {};
		}

		const s64 valueAt = offset + slotSpan(keyHeader);
		String key(static_cast<size_t>(keyHeader.size), '\0');
		archive->seek(valueAt - keyHeader.size);
		archive->serializeBytes(reinterpret_cast<ptr<u8>>(key.data()), key.size(), resource::ArchiveStreamMode::eLoad);
		if (not checkArchive(offset, slotSpan(keyHeader))) {
			return {};
		}

		StructureSlotHeader valueHeader {};
		if (not readHeaderAt(valueAt, end, valueHeader)) {
			return {};
		}

		out.emplace_back(::hg::move(key));
		offset = valueAt + slotSpan(valueHeader);
	}
	return out;
}

void StructSlot::warn(const ReadError::Kind kind_, String detail_) const {
	StructureSlotBase::warn(kind_, _state.offset, slotSpan(_state.header), ::hg::move(detail_));
}

bool StructSlot::reportUnreadKeys() const {

	if (not hasPayload() || not _state.root->context) {
		return false;
	}

	auto present = keys();
	std::erase_if(present, [](ref<const String> key_) { return key_ == Symbols::Type.str || key_ == Symbols::Revision.str; });

	const auto unread = _state.root->context->unreadKeys(&*_state.root->archive, _state.offset, present);
	for (const auto& key : unread) {
		warn(ReadError::Kind::eUnknownField, "key `" + key + "` was not read");
	}
	return not unread.empty();
}

#include "StructSlot.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Resource.Archive/Archive.hpp>
#include <Engine.Resource.Archive/ArchiveStreamMode.hpp>

#include "RecordSlot.hpp"
#include "StringSlot.hpp"

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

#include "SeqSlot.hpp"

#include <Engine.Common/__macro.hpp>
#include <Engine.Resource.Archive/Archive.hpp>
#include <cassert>

#include "RecordSlot.hpp"

import Engine.Common;

using namespace hg::engine::serialization;
using namespace hg;

SeqSlot::SeqSlot(cref<StructureSlotState> state_) noexcept :
	StructureSlotBase(state_) {
	_state.header = StructureSlotHeader::from<StructureSlotType::eSeq>();
}

SeqSlot::SeqSlot(mref<StructureSlotState> state_) noexcept :
	StructureSlotBase(std::move(state_)) {
	_state.header = StructureSlotHeader::from<StructureSlotType::eSeq>();
}

StructureSlotType SeqSlot::getSlotType() const noexcept {
	return StructureSlotType::eSeq;
}

bool SeqSlot::validateType() const noexcept {
	return _state.header.type == StructureSlotType::eSeq;
}

void SeqSlot::feedback(const non_owning_rptr<const StructureSlotBase> other_) {

	const auto& header = other_->getSlotHeader();
	s64 add = header.size + sizeof(StructureSlotHeader::type);

	if (not header.sizeInferred) {
		add += sizeof(StructureSlotHeader::size);
	}

	/**/

	_state.header.size = MAX(_state.header.size, 0) + add;
	writeHeader();
}

sptr<RecordSlot> SeqSlot::addRecord() {

	_state.flags |= StructureSlotStateFlag::eDirty;
	auto archive = _state.root->archive;

	/**/

	constexpr s64 off = sizeof(StructureSlotHeader::type) + sizeof(StructureSlotHeader::size);

	u64 count = 0uLL;
	if (_state.header.size > sizeof(u64)) {

		archive->seek(_state.offset + off);
		(*archive) >> count;

	} else {
		_state.header.size += sizeof(count);
	}

	++count;

	archive->seek(_state.offset + off);
	(*archive) << count;

	/**/

	s64 offset = _state.offset + off;

	if (_state.header.size > sizeof(u64)) {
		offset += _state.header.size;

	} else {

		constexpr s64 ioff = sizeof(count);
		offset += ioff;
	}

	/**/

	StructureSlotState state {
		StructureSlotStateFlags { StructureSlotStateFlag::eMutable } | StructureSlotStateFlag::eDirty,
		offset,
		StructureSlotHeader::from<StructureSlotType::eRecord>(),
		shared_from_this(),
		_state.root
	};

	return make_sptr<RecordSlot>(std::move(state));
}

sptr<RecordSlot> SeqSlot::getRecord(const u64 index_) {

	const auto empty = [this] {
		return make_sptr<RecordSlot>(
			StructureSlotState {
				StructureSlotStateFlag::eImmutable, -1LL, StructureSlotHeader::from<StructureSlotType::eRecord>(), shared_from_this(),
				_state.root
			}
		);
	};

	const auto count = getRecordCount();
	if (count <= 0LL || index_ >= static_cast<u64>(count)) {
		return empty();
	}

	/**/

	const s64 end = payloadEnd();
	s64 offset = payloadBegin() + static_cast<s64>(sizeof(u64));

	for (u64 i = 0; i != index_; ++i) {
		StructureSlotHeader header {};
		if (not readHeaderAt(offset, end, header)) {
			return empty();
		}
		offset += slotSpan(header);
	}

	if (offset >= end) {
		fail(ReadError::Kind::eMalformed, offset, 0LL, "seq record past the end of its parent");
		return empty();
	}

	/**/

	StructureSlotState state {
		StructureSlotStateFlag::eImmutable,
		offset,
		StructureSlotHeader::from<StructureSlotType::eRecord>(),
		shared_from_this(),
		_state.root
	};

	return make_sptr<RecordSlot>(std::move(state));
}

s64 SeqSlot::getRecordCount() {

	if (not hasPayload() || _state.header.size == 0LL) {
		return 0LL;
	}

	if (_state.header.type != StructureSlotType::eSeq) {
		fail(ReadError::Kind::eTypeMismatch, _state.offset, slotSpan(_state.header), "slot is not a seq");
		return -1LL;
	}

	if (_state.header.size < static_cast<s64>(sizeof(u64))) {
		fail(ReadError::Kind::eMalformed, _state.offset, slotSpan(_state.header), "seq too small for its count");
		return -1LL;
	}

	/**/

	auto archive = _state.root->archive;
	archive->seek(payloadBegin());

	u64 storedCount = 0uLL;
	(*archive) >> storedCount;

	if (not checkArchive(_state.offset, slotSpan(_state.header))) {
		return -1LL;
	}

	if (storedCount > static_cast<u64>(_state.header.size) - sizeof(u64)) {
		fail(ReadError::Kind::eMalformed, payloadBegin(), sizeof(u64), "seq count exceeds its bytes");
		return -1LL;
	}

	return static_cast<s64>(storedCount);
}

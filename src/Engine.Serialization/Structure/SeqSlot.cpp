#include "SeqSlot.hpp"

#include <cassert>
#include <Engine.Common/Make.hpp>
#include <Engine.Resource.Archive/Archive.hpp>

#include "RecordSlot.hpp"

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

	if (_state.header.size <= sizeof(u64)) {
		return nullptr;
	}

	/**/

	auto archive = _state.root->archive;

	constexpr s64 off = sizeof(StructureSlotHeader::type) + sizeof(StructureSlotHeader::size);
	archive->seek(_state.offset + off);

	/**/

	u64 storedCount = ~0uLL;
	(*archive) >> storedCount;

	assert(storedCount != ~0uLL);

	/**/

	if (index_ >= storedCount) {
		return nullptr;
	}

	/**/

	constexpr s64 ioff = sizeof(storedCount);

	const s64 end = _state.offset + off + _state.header.size;

	u64 i = 0;
	s64 offset = _state.offset + off + ioff;

	while (i != index_ && offset < end) {

		archive->seek(offset);

		StructureSlotType storedType = StructureSlotType::eUndefined;
		(*archive) >> storedType;

		auto storedHeader = StructureSlotHeader::from(storedType);
		if (not storedHeader.sizeInferred) {
			(*archive) >> storedHeader.size;
		}

		/**/

		offset += storedHeader.size;
		offset += sizeof(StructureSlotHeader::type);

		if (not storedHeader.sizeInferred) {
			offset += sizeof(StructureSlotHeader::size);
		}

		/**/
		++i;
	}

	/**/

	if (i != index_ || offset >= end) {
		return nullptr;
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

	if (_state.header.size <= sizeof(u64)) {
		return -1LL;
	}

	/**/

	auto archive = _state.root->archive;

	constexpr s64 off = sizeof(StructureSlotHeader::type) + sizeof(StructureSlotHeader::size);
	archive->seek(_state.offset + off);

	/**/

	u64 storedCount = ~0uLL;
	(*archive) >> storedCount;

	return storedCount != ~0uLL ? static_cast<s64>(storedCount) : -1LL;
}

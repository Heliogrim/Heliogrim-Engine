#include "RecordScopedSlot.hpp"

#include "RecordSlot.hpp"
#include "SeqScopedSlot.hpp"
#include "StructScopedSlot.hpp"

import Engine.Common;

using namespace hg::engine::serialization;
using namespace hg;

RecordScopedSlot::RecordScopedSlot(mref<ScopedSlotState> scopedState_, mref<sptr<RecordSlot>> slot_) :
	ScopedSlot(std::move(scopedState_), slot_ ? std::move(slot_) : make_sptr<RecordSlot>(StructureSlotState {})) {}

RecordScopedSlot::RecordScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_) :
	ScopedSlot(std::move(scopedState_), make_sptr<RecordSlot>(std::move(state_))) {}

RecordScopedSlot::~RecordScopedSlot() = default;

StructScopedSlot RecordScopedSlot::intoStruct() && {
	return StructScopedSlot { std::move(_state), std::move(_slot->getState()) };
}

StructScopedSlot RecordScopedSlot::asStruct() const {

	StructureSlotState aliasState { _slot->getState() };
	aliasState.flags = StructureSlotStateFlag::eImmutable;

	return StructScopedSlot { ScopedSlotState { _state }, std::move(aliasState) };
}

SeqScopedSlot RecordScopedSlot::intoSeq() && {
	return SeqScopedSlot { std::move(_state), std::move(_slot->getState()) };
}

SeqScopedSlot RecordScopedSlot::asSeq() const {

	StructureSlotState aliasState { _slot->getState() };
	aliasState.flags = StructureSlotStateFlag::eImmutable;

	return SeqScopedSlot { ScopedSlotState { _state }, std::move(aliasState) };
}

StringScopedSlot RecordScopedSlot::intoString() && {
	return StringScopedSlot { std::move(_state), std::move(_slot->getState()) };
}

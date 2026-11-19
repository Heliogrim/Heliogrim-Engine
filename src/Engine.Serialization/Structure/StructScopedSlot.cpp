#include "StructScopedSlot.hpp"

#include <Engine.Common/__macro.hpp>

#include "IntegralScopedSlot.hpp"
#include "StructSlot.hpp"
#include "StructureSlotTypeTraits.hpp"

import Engine.Asserts;
import Engine.Common;

using namespace hg::engine::serialization;
using namespace hg;

StructScopedSlot::StructScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_) :
	ScopedSlot(std::move(scopedState_), make_sptr<StructSlot>(std::move(state_))) {

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

u16 StructScopedSlot::revision() const noexcept {
	u16 value = 0;
	if (slot() && slot()->hasPayload()) {
		getSlot<u16>("@rev") >> value;
	}
	return value;
}

void StructScopedSlot::warn(const ReadError::Kind kind_, String detail_) const {
	if (slot()) {
		static_cast<ptr<StructSlot>>(slot())->warn(kind_, std::move(detail_));
	}
}

bool StructScopedSlot::reportUnreadKeys() const {
	return slot() && static_cast<ptr<StructSlot>>(slot())->reportUnreadKeys();
}

RecordScopedSlot StructScopedSlot::insertRecordSlot(cref<record_key_type> key_) {

	auto record = static_cast<ptr<StructSlot>>(slot())->insertRecord(key_);
	ScopedSlotState scopedState { ScopedSlotStateFlag::eDirty };

	return RecordScopedSlot { std::move(scopedState), std::move(record) };
}

StructScopedSlot StructScopedSlot::insertStructSlot(cref<record_key_type> key_) {
	return insertRecordSlot(key_).intoStruct();
}

bool StructScopedSlot::hasRecordSlot(cref<record_key_type> key_) const {
	return static_cast<ptr<StructSlot>>(slot())->hasRecord(key_);
}

RecordScopedSlot StructScopedSlot::getRecordSlot(cref<record_key_type> key_) const {

	auto record = static_cast<ptr<StructSlot>>(slot())->getRecord(key_);
	ScopedSlotState scopedState { ScopedSlotStateFlag::eClean };

	return RecordScopedSlot { std::move(scopedState), std::move(record) };
}

RecordScopedSlot StructScopedSlot::getRecordSlot(const size_t index_) const {
	::hg::todo_panic();
}

StructScopedSlot StructScopedSlot::getStructSlot(cref<record_key_type> key_) const {
	// Note: We need to use into, as `asStruct` would make the returned slot immutable.
	return getRecordSlot(key_).intoStruct();
}

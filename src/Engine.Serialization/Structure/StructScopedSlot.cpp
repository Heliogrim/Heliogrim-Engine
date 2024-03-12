#include "StructScopedSlot.hpp"

#include <Engine.Asserts/Todo.hpp>

#include "StructureSlotTypeTraits.hpp"
#include "StructSlot.hpp"
#include <Engine.Common/Make.hpp>

#ifdef _DEBUG
#include <Engine.Logging/Logger.hpp>
#endif

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

RecordScopedSlot StructScopedSlot::insertRecordSlot(cref<record_key_type> key_) {

	auto record = static_cast<ptr<StructSlot>>(slot())->insertRecord(key_);
	ScopedSlotState scopedState { ScopedSlotStateFlag::eDirty };

	return RecordScopedSlot { std::move(scopedState), std::move(record) };
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

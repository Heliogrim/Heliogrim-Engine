#include "RootSlot.hpp"

#include <Engine.Common/Sal.hpp>
#include <Engine.Resource.Archive/Archive.hpp>

import Engine.Common;

using namespace hg::engine::serialization;
using namespace hg;

RootSlot::RootSlot(cref<StructureSlotState> state_) :
	RecordSlot(state_) {}

RootSlot::RootSlot(mref<StructureSlotState> state_) :
	RecordSlot(std::move(state_)) {}

RootSlot::RootSlot(_Inout_ ref<resource::Archive> archive_) :
	RecordSlot(makeRootState(archive_)) {}

StructureSlotState RootSlot::makeRootState(_In_ ref<resource::Archive> archive_, nmpt<LoadContext> context_) {
	return StructureSlotState {
		.flags = StructureSlotStateFlag::eUndefined,
		.offset = archive_.tell(),
		.header = StructureSlotHeader {},
		.parent = nullptr,
		.root = make_sptr<RootStructureSlotState>(
			RootStructureSlotState {
				StructureSlotStateFlag::eUndefined,
				archive_.tell(),
				StructureSlotHeader {},
				nullptr,
				nullptr,
				std::addressof(archive_),
				context_
			}
		)
	};
}

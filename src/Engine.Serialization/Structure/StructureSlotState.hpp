#pragma once

#include "__fwd.hpp"
#include "StructureSlotHeader.hpp"
#include "StructureSlotStateFlags.hpp"
#include "../Archive/__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::serialization {
	struct StructureSlotState {
		StructureSlotStateFlags flags = StructureSlotStateFlag::eUndefined;
		s64 offset = -1;
		//
		StructureSlotHeader header;
		//
		sptr<StructureSlotBase> parent;
		sptr<struct RootStructureSlotState> root;
	};

	struct RootStructureSlotState final :
		public StructureSlotState {
		nmpt<resource::Archive> archive = nullptr;
		nmpt<class LoadContext> context = nullptr;
		/* Set on scratch roots of migrated records: errors report the original record, not scratch offsets. */
		s64 originOffset = -1;
		s64 originLength = 0;
	};
}

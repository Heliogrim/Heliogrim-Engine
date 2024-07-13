#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "../Archive/__fwd.hpp"
#include "__fwd.hpp"
#include "StructureSlotStateFlags.hpp"
#include "StructureSlotHeader.hpp"
#include "Engine.Common/Memory/MemoryPointer.hpp"

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
	};
}

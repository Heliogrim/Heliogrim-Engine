#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::serialization {
	struct StructureSlotState;
	class StructureSlotBase;
}

namespace hg::engine::serialization {
	enum class ScopedSlotStateFlag : bool {
		eUndefined = false,
		/**/
		eClean = eUndefined,
		eDirty = true
	};

	typedef Flag<ScopedSlotStateFlag, bool> ScopedSlotStateFlags;

	struct ScopedSlotState {
		ScopedSlotStateFlags flags;
	};
}

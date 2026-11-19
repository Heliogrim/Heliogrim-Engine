#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::serialization {
	enum class StructureSlotStateFlag : u8 {
		eUndefined = 0x0,
		/**/
		eMutable = 0x1,
		eImmutable = 0x2,
		/**/
		eDirty = 0x4
	};

	typedef Flag<StructureSlotStateFlag, u8> StructureSlotStateFlags;
}

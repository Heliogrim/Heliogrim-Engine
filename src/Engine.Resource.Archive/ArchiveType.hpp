#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::resource {
	enum class ArchiveType : u8 {
		eUndefined = 0x0,
		//
		eSerializedStructure = 0x2,
		eSerializedLayout = 0x3,
		//
		eRaw = 0xFF,
	};

	static_assert(sizeof(ArchiveType) == 1uLL);
}

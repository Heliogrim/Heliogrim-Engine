#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::resource {
	#pragma pack(push)
	#pragma pack(1)
	struct LinkedDataSpan {
		streamoff offset;
		streamsize size;
	};
	#pragma pack(pop)

	static_assert(std::is_trivial_v<LinkedDataSpan>);
	static_assert(sizeof(LinkedDataSpan) == 16uLL);
}

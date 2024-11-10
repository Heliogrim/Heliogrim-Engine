#pragma once

#include <Engine.Common/Types.hpp>

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

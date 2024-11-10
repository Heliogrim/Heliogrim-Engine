#pragma once

#include "LinkedDataSpan.hpp"
#include "PackageArchiveHeader.hpp"

namespace hg::engine::resource::package {
	#pragma push
	#pragma pack(1)
	struct Indexed {
		LinkedDataSpan data;
		PackageArchiveHeader header;
	};
	#pragma pop

	static_assert(sizeof(Indexed) == (8 + 8 + 1 + 15));
}

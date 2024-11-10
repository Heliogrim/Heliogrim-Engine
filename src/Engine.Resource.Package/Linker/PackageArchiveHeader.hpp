#pragma once

#include "ArchiveGuid.hpp"
#include "PackageArchiveType.hpp"

namespace hg::engine::resource::package {
	#pragma pack(push)
	#pragma pack(1)
	struct PackageArchiveHeader {
		PackageArchiveType type;
		ArchiveGuid guid;
	};
	#pragma pack(pop)

	static_assert(sizeof(PackageArchiveHeader) == 16uLL);
}

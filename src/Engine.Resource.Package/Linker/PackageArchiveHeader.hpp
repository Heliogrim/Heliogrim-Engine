#pragma once

#include <Engine.Resource.Archive/ArchiveGuid.hpp>
#include <Engine.Resource.Archive/ArchiveType.hpp>

namespace hg::engine::resource::package {
	#pragma pack(push)
	#pragma pack(1)
	struct PackageArchiveHeader {
		ArchiveType type;
		ArchiveGuid guid;
	};
	#pragma pack(pop)

	static_assert(sizeof(PackageArchiveHeader) == 16uLL);
}

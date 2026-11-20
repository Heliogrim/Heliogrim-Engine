#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Resource.Archive;
#endif

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

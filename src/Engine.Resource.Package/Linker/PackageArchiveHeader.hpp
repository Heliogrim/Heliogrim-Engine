#pragma once

#include <Engine.Common/Guid.hpp>

#include "PackageArchiveType.hpp"

namespace hg::engine::resource::package {
	#pragma pack(push)
	#pragma pack(1)
	struct PackageArchiveHeader {
		PackageArchiveType type;
		Guid guid;
	};
	#pragma pack(pop)
}

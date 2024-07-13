#pragma once

#include "PackageArchiveHeader.hpp"
#include "../Index/PackageIndexEntry.hpp"

namespace hg::engine::resource::package {
	struct LinkedArchive {
		PackageArchiveHeader header;
		PackageIndexEntry index;
	};
}

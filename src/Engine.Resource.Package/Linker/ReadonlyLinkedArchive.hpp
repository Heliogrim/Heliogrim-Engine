#pragma once

#include "LinkedDataSpan.hpp"
#include "PackageArchiveHeader.hpp"

namespace hg::engine::resource::package {
	struct ReadonlyLinkedArchive {
		LinkedDataSpan data;
		PackageArchiveHeader header;
	};
}

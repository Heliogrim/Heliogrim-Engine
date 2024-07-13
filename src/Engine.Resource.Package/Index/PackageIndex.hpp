#pragma once

#include <span>

#include "PackageIndexEntry.hpp"

namespace hg::engine::resource {
	class PackageIndex :
		public std::span<PackageIndexEntry> {
	public:
		using this_type = PackageIndex;
		using std::span<PackageIndexEntry>::span;
	};
}

#pragma once

namespace hg::engine::resource {
	class Package;

	class PackageIndex;
	struct PackageIndexEntry;
}

namespace hg::engine::resource::package {
	class PackageLinker;

	struct PackageArchiveHeader;
	struct PackageLinkedArchive;

	struct LinkedArchive;

	template <bool Const_>
	class LinkedArchiveIterator;
}

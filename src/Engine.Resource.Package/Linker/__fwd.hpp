#pragma once

namespace hg::engine::resource {
	class Package;
	class ReadonlyPackage;
	class ReadWritePackage;

	class PackageLinker;
	class ReadonlyPackageLinker;
	class ReadWritePackageLinker;
}

namespace hg::engine::resource::package {
	class ArchiveGuid;

	class LinkedDataSpan;
	struct PackageArchiveHeader;

	struct ReadonlyLinkedArchive;
	struct ReadWriteLinkedArchive;
}

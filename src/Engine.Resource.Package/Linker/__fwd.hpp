#pragma once

namespace hg::engine::resource {
	class Package;
	class ReadonlyPackage;
	class ReadWritePackage;

	class PackageLinker;
	class ReadonlyPackageLinker;
	class ReadWritePackageLinker;

	class ArchiveGuid;
}

namespace hg::engine::resource::package {
	class LinkedDataSpan;
	struct PackageArchiveHeader;

	struct ReadonlyLinkedArchive;
	struct ReadWriteLinkedArchive;
}

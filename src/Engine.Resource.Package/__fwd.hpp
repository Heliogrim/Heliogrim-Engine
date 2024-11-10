#pragma once

namespace hg::engine::resource {
	class Package;
	class ReadonlyPackage;
	class ReadWritePackage;

	struct PackageHeader;
	struct PackageFooter;

	class PackageLinker;
	class ReadonlyPackageLinker;
	class ReadWritePackageLinker;
}

namespace hg::engine::storage {
	class PackageIo;
}

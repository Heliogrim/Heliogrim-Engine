#include "Pattern.hpp"

#include <cstring>

#include <Engine.Resource.Package/Attribute/MagicBytes.hpp>
#include <Engine.Resource.Package/Footer/PackageFooter.hpp>
#include <Engine.Resource.Package/Header/PackageHeader.hpp>
#include <Engine.Resource.Package/Linker/PackageArchiveHeader.hpp>

using namespace hg::engine::storage::package;
using namespace hg::engine::resource::package;
using namespace hg::engine::resource;
using namespace hg;

bool engine::storage::package::isPackageBlob(ref<const Blob> blob_) {

	constexpr auto minimumPackageSize = sizeof(PackageHeader) + sizeof(PackageFooter);

	auto header = PackageHeader {};
	blob_.read(static_cast<streamoff>(offsetof(PackageHeader, magicBytes)), header.magicBytes);

	return blob_.size() >= minimumPackageSize && std::memcmp(
		header.magicBytes.data(),
		PackageMagicBytes.data(),
		PackageMagicBytes.size()
	) == 0;
}

bool engine::storage::package::isPackageFileUrl(cref<FileUrl> fileUrl_) {

	const auto fileUrlExt = static_cast<std::filesystem::path>(fileUrl_.path()).extension();
	if (fileUrlExt.empty()) {
		return false;
	}

	auto strExt = fileUrlExt.string();
	if (strExt.empty() || (not strExt.ends_with(".imp"sv) && not strExt.ends_with(".impackage"sv))) {
		return false;
	}

	return true;
}

bool engine::storage::package::isPackageFile(cref<FileUrl> fileUrl_, ref<const Blob> blob_) {
	return isPackageFileUrl(fileUrl_) && isPackageBlob(blob_);
}

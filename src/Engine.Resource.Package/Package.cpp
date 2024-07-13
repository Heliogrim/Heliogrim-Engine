#include "Package.hpp"

#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Resource.Blob/Blob.hpp>

#include "Linker/PackageLinker.hpp"

using namespace hg::engine::resource;
using namespace hg;

Package::Package(
	ref<storage::AccessBlobReadWrite> blobAccessor_,
	mref<PackageHeader> header_,
	mref<PackageFooter> footer_
) :
	_blob(std::addressof(blobAccessor_)),
	_header(std::move(header_)),
	_footer(std::move(footer_)) {}

Package::Package(
	ref<storage::AccessBlobReadonly> blobAccessor_,
	mref<PackageHeader> header_,
	mref<PackageFooter> footer_
) :
	_blob(std::addressof(blobAccessor_)),
	_header(std::move(header_)),
	_footer(std::move(footer_)) {}

Package::~Package() = default;

cref<PackageHeader> Package::header() const noexcept {
	return _header;
}

ref<PackageHeader> Package::header() noexcept {
	return _header;
}

cref<PackageFooter> Package::footer() const noexcept {
	return _footer;
}

ref<PackageFooter> Package::footer() noexcept {
	return _footer;
}

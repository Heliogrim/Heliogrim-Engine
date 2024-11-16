#include "ReadonlyPackageLinker.hpp"

#include <Engine.Common/Move.hpp>

using namespace hg::engine::resource::package;
using namespace hg::engine::resource;
using namespace hg;

ReadonlyPackageLinker::ReadonlyPackageLinker(ref<const Blob> blob_, mref<link_container_type> links_) noexcept :
	PackageLinker(),
	_blob(blob_),
	_links(::hg::move(links_)) {}

ref<const Blob> ReadonlyPackageLinker::getStorage() const noexcept {
	return _blob;
}

streamsize ReadonlyPackageLinker::computeArchiveSize() const noexcept {
	streamsize size {};
	for (const auto& linked : _links.values()) {
		size += linked.second.data.size;
	}
	return size;
}

bool ReadonlyPackageLinker::contains(cref<archive_guid_type> guid_) const noexcept {
	return _links.contains(guid_);
}

size_t ReadonlyPackageLinker::count() const noexcept {
	return _links.size();
}

Opt<ReadonlyPackageLinker::archive_link_type> ReadonlyPackageLinker::get(cref<Guid> guid_) const noexcept {
	auto it = _links.find(ArchiveGuid::from(guid_));
	return it != _links.end() ? Some(it->second) : None;
}

Opt<ReadonlyPackageLinker::archive_link_type> ReadonlyPackageLinker::get(cref<ArchiveGuid> archiveGuid_) const noexcept {
	auto it = _links.find(archiveGuid_);
	return it != _links.end() ? Some(it->second) : None;
}

ReadonlyPackageLinker::iterator_type ReadonlyPackageLinker::begin() const noexcept {
	return ReadonlyPackageIterator { _links.begin() };
}

ReadonlyPackageLinker::iterator_type ReadonlyPackageLinker::end() const noexcept {
	return ReadonlyPackageIterator { _links.end() };
}

ReadonlyPackageLinker::iterator_type ReadonlyPackageLinker::find(cref<archive_guid_type> guid_) const noexcept {
	return ReadonlyPackageIterator { _links.find(guid_) };
}

Opt<ReadonlyPackageLinker::archive_link_type> ReadonlyPackageLinker::operator[](size_t index_) const noexcept {
	return (_links.values().size() > index_) ? Some(_links.values()[index_].second) : None;
}

Opt<ReadonlyPackageLinker::archive_link_type> ReadonlyPackageLinker::operator[](cref<archive_guid_type> guid_) const noexcept {
	return get(guid_);
}

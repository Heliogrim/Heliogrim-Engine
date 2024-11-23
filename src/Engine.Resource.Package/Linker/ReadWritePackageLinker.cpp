#include "ReadWritePackageLinker.hpp"

#include <algorithm>
#include <memory>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Asserts/Todo.hpp>
#include <Engine.Common/Move.hpp>

#include "Indexed.hpp"

using namespace hg::engine::resource::package;
using namespace hg::engine::resource;
using namespace hg;

ReadWritePackageLinker::ReadWritePackageLinker(ref<Blob> blob_, mref<link_container_type> links_) noexcept :
	PackageLinker(),
	_blob(blob_),
	_links(::hg::move(links_)) {}

ref<const Blob> ReadWritePackageLinker::getStorage() const noexcept {
	return _blob;
}

ref<Blob> ReadWritePackageLinker::getStorage() noexcept {
	return _blob;
}

streamsize ReadWritePackageLinker::computeArchiveSize() const noexcept {

	// TODO: Check whether we need to take the upcomming changes into account.
	auto size = streamsize {};
	for (const auto& linked : _links) {
		size += linked.second.data.size;
	}
	return size;
}

streamsize ReadWritePackageLinker::computeIndexSize() const noexcept {

	// TODO: Check whether we need to take the upcomming changes into account.
	return static_cast<streamsize>(_links.size() * sizeof(Indexed));
}

ref<ReadWritePackageLinker::archive_link_type> ReadWritePackageLinker::add(mref<PackageArchiveHeader> header_) {

	::hg::assertrt(not _links.contains(header_.guid));

	const auto [it, success] = _links.emplace(header_.guid, archive_link_type { LinkedDataSpan {}, ::hg::move(header_) });
	::hg::assertd(success);

	return it->second;
}

bool ReadWritePackageLinker::contains(cref<Guid> guid_) const noexcept {
	return _links.contains(ArchiveGuid::from(guid_));
}

size_t ReadWritePackageLinker::count() const noexcept {
	return _links.size();
}

Opt<ref<const ReadWritePackageLinker::archive_link_type>> ReadWritePackageLinker::get(cref<Guid> guid_) const noexcept {
	return get(ArchiveGuid::from(guid_));
}

Opt<ref<const ReadWritePackageLinker::archive_link_type>> ReadWritePackageLinker::get(cref<ArchiveGuid> archiveGuid_) const noexcept {
	auto it = _links.find(archiveGuid_);
	return it != _links.end() ? Some(it->second) : None;
}

Opt<ref<ReadWritePackageLinker::archive_link_type>> ReadWritePackageLinker::get(cref<ArchiveGuid> archiveGuid_) noexcept {
	auto it = _links.find(archiveGuid_);
	return it != _links.end() ? Some(it->second) : None;
}

void ReadWritePackageLinker::drop(cref<ArchiveGuid> archiveGuid_) noexcept {
	// TODO:
	// Question: Are we safe to assume that dropping a archive from this package linker, will not result in use-after-free?
	_links.erase(archiveGuid_);
}

ReadWritePackageLinker::iterator_type ReadWritePackageLinker::begin() const noexcept {
	return ReadWritePackageIterator { _links.begin() };
}

ReadWritePackageLinker::mutable_iterator_type ReadWritePackageLinker::begin() noexcept {
	return MutableReadWritePackageIterator { _links.begin() };
}

ReadWritePackageLinker::iterator_type ReadWritePackageLinker::end() const noexcept {
	return ReadWritePackageIterator { _links.end() };
}

ReadWritePackageLinker::mutable_iterator_type ReadWritePackageLinker::end() noexcept {
	return MutableReadWritePackageIterator { _links.end() };
}

ReadWritePackageLinker::iterator_type ReadWritePackageLinker::find(cref<Guid> guid_) const noexcept {
	return find(ArchiveGuid::from(guid_));
}

ReadWritePackageLinker::iterator_type ReadWritePackageLinker::find(cref<ArchiveGuid> archiveGuid_) const noexcept {
	return ReadWritePackageIterator { _links.find(archiveGuid_) };
}

Opt<ref<const ReadWritePackageLinker::archive_link_type>> ReadWritePackageLinker::operator[](size_t index_) const noexcept {
	if (_links.size() <= index_) {
		return None;
	}

	auto it = _links.begin();
	std::advance(it, index_);
	return (it != _links.end() && it->first != ArchiveGuid {}) ? Some(it->second) : None;
}

Opt<ref<const ReadWritePackageLinker::archive_link_type>> ReadWritePackageLinker::operator[](cref<Guid> guid_) const noexcept {
	return get(guid_);
}

Opt<ref<const ReadWritePackageLinker::archive_link_type>> ReadWritePackageLinker::operator[
](cref<ArchiveGuid> archiveGuid_) const noexcept {
	return get(archiveGuid_);
}

Opt<ref<ReadWritePackageLinker::archive_link_type>> ReadWritePackageLinker::operator[](cref<ArchiveGuid> archiveGuid_) noexcept {
	return get(archiveGuid_);
}

bool ReadWritePackageLinker::hasChanges() const noexcept {
	return std::ranges::any_of(_links, [](const auto& link_) { return not link_.second.changes.empty(); });
}

Vector<nmpt<ReadWritePackageLinker::archive_link_type>> ReadWritePackageLinker::computeChangeList() noexcept {

	auto changeList = Vector<nmpt<archive_link_type>> {};
	for (auto& [key, value] : _links) {
		if (not value.changes.empty()) {
			changeList.emplace_back(std::addressof(value));
		}
	}

	return changeList;
}

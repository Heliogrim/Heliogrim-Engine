#include "PackageLinker.hpp"

#include <corecrt_io.h>
#include <Engine.Common/Make.hpp>
#include <Engine.Resource.Archive/StorageReadonlyArchive.hpp>
#include <Engine.Resource.Archive/StorageReadWriteArchive.hpp>

#include "LinkedArchiveIterator.hpp"
#include "Engine.Asserts/Todo.hpp"
#include "Engine.Common/Functional/Lambda.hpp"
#include "Engine.Resource.Archive/BufferArchive.hpp"
#include "Engine.Resource.Package/Package.hpp"
#include <Engine.Storage.IO/StorageIo.hpp>

using namespace hg::engine::resource::package;
using namespace hg::engine::resource;
using namespace hg;

PackageLinker::PackageLinker(
	cref<storage::StorageIo> storageIo_,
	ref<storage::AccessPackageReadWrite> packageAccessor_
) :
	_storageIo(std::addressof(storageIo_)),
	_package(std::addressof(packageAccessor_)),
	_links() {}

PackageLinker::PackageLinker(
	cref<storage::StorageIo> storageIo_,
	ref<storage::AccessPackageReadonly> packageAccessor_
) :
	_storageIo(std::addressof(storageIo_)),
	_package(std::addressof(packageAccessor_)),
	_links() {}

PackageLinker::~PackageLinker() = default;

bool PackageLinker::canLoad() const noexcept {
	return not _package.valueless_by_exception();
}

bool PackageLinker::canStore() const noexcept {
	return _package.is<nmpt<storage::AccessPackageReadWrite>>();
}

bool PackageLinker::store(mref<PackageArchiveHeader> header_, mref<uptr<Archive>> archive_) {

	::hg::assertrt(canStore());

	constexpr streamoff baseOffset = sizeof(PackageHeader);
	streamoff nextBaseOffset = { baseOffset };

	if (not _links.empty()) {
		const auto lastIndex = _links.back().index;
		nextBaseOffset = static_cast<streamoff>(lastIndex.offset + lastIndex.size);
	}

	/**/

	auto& package = _package.as<nmpt<storage::AccessPackageReadWrite>>()->get().get().fully();
	auto& blob = package._blob.as<nmpt<storage::AccessBlobReadWrite>>()->get().get().fully();

	/**/

	/* Write Archive Header to Package */

	BufferArchive headerArchive {};
	headerArchive.reserve(sizeof(PackageArchiveHeader));

	// Error: we need to use hton / ntoh to prepare guid to get stored correctly over multiple platforms
	headerArchive.serializeBytes(std::addressof(header_.guid), sizeof(header_.guid), ArchiveStreamMode::eStore);
	headerArchive << header_.type;

	blob.write(nextBaseOffset, headerArchive.getByteSpan());

	/* Write Archive to Package */

	const auto archive = std::move(archive_);
	const auto archiveSize = archive->totalSize();

	archive->seek(0);

	_::byte buffer[1024];
	streamsize done = 0;
	streamoff nextContentOffset = nextBaseOffset + static_cast<streamoff>(sizeof(PackageArchiveHeader));

	while (not archive->isEOA()) {

		const streamsize left = archiveSize - done;
		const streamsize chunkSize = left > 1024 ? 1024 : left;

		// Warning: This is not intended to be used this way
		archive->seek(done);
		archive->serializeBytes(buffer, chunkSize, ArchiveStreamMode::eLoad);

		blob.write(
			nextContentOffset + done,
			std::span<_::byte> { &buffer[0], static_cast<size_t>(chunkSize) }
		);

		done += chunkSize;
	}

	/**/

	_links.emplace_back(
		std::move(header_),
		PackageIndexEntry {
			nextBaseOffset,
			archiveSize + static_cast<streamsize>(sizeof(PackageArchiveHeader))
		}
	);

	return false;
}

PackageLinker::tracked_locked_writer PackageLinker::store(mref<PackageArchiveHeader> header_, std::try_to_lock_t) {

	::hg::assertrt(canStore());

	constexpr streamoff baseOffset = sizeof(PackageHeader);
	streamoff nextOffset = { baseOffset };

	if (not _links.empty()) {
		const auto lastIndex = _links.back().index;
		nextOffset = static_cast<streamoff>(lastIndex.offset + lastIndex.size);
	}

	/**/

	auto& package = _package.as<nmpt<storage::AccessPackageReadWrite>>()->get().get().fully();
	auto& blob = package._blob.as<nmpt<storage::AccessBlobReadWrite>>()->get().get().fully();

	/**/

	// TODO: Lock linker, to prevent concurrent mutation

	/* Write Archive Header to Package */

	BufferArchive headerArchive {};
	headerArchive.reserve(sizeof(PackageArchiveHeader));

	// Error: we need to use hton / ntoh to prepare guid to get stored correctly over multiple platforms
	headerArchive.serializeBytes(std::addressof(header_.guid), sizeof(header_.guid), ArchiveStreamMode::eStore);
	headerArchive << header_.type;

	blob.write(nextOffset, headerArchive.getByteSpan());

	/**/

	// Note: Preserve Link entry, fixup later
	_links.emplace_back(
		std::move(header_),
		PackageIndexEntry {
			nextOffset,
			/* archiveSize + */ static_cast<streamsize>(sizeof(PackageArchiveHeader))
		}
	);

	/**/

	// TODO: Rework accessor ownership
	//const auto capacity = blob.capacity() - nextOffset;
	// Warning: Random magic number...
	const streamsize capacity = 1024LL;

	auto* rwa = make_ptr<StorageReadWriteArchive>(
		// Note: We move the accessor ownership from the underlying package into the exposed writer
		std::move(*package._blob.as<nmpt<storage::AccessBlobReadWrite>>()),
		nextOffset + sizeof(PackageArchiveHeader),
		clone(capacity)
	);
	const auto initialSize = rwa->totalSize();

	return tracked_locked_writer {
		std::move(rwa),
		[this, /* Problem: This is just a quick fix */initialSize](ptr<StorageReadWriteArchive> obj_) {

			// Problem: This is just an assumption
			const auto finalizeSize = obj_->totalSize();
			const auto writtenBytes = finalizeSize - initialSize;

			// Note: After RAII finalization triggers, we need to restore the package's blob accessor
			auto& package = _package.as<nmpt<storage::AccessPackageReadWrite>>()->get().get().fully();
			*package._blob.as<nmpt<storage::AccessBlobReadWrite>>() = std::move(*obj_).release();

			/**/

			// Question: Are a able to guarantee, that the linker only adds links strictly sequential?
			auto& link = _links.back();
			link.index.size += writtenBytes;

			/**/

			delete obj_;
		}
	};
}

uptr<StorageReadonlyArchive> PackageLinker::load(mref<Guid> archiveGuid_) const noexcept {

	const auto where = std::ranges::find(
		_links,
		archiveGuid_,
		[](cref<LinkedArchive> linked_) {
			return linked_.header.guid;
		}
	);

	if (where == _links.end()) {
		return nullptr;
	}

	constexpr auto archiveHeaderSize = static_cast<streamsize>(sizeof(PackageArchiveHeader));

	// Error: This will only work when there is no compression applied... TBD: Proxy stream compression handling

	// TODO: Get package storage object from provided package accessor.
	auto blobAccess = _storageIo->accessReadonlyBlob(Arci<storage::IStorage> {});

	const auto index = where->index;
	return make_uptr<StorageReadonlyArchive>(
		std::move(blobAccess),
		index.offset + archiveHeaderSize,
		index.size - archiveHeaderSize
	);
}

bool PackageLinker::containsArchive(mref<Guid> archiveGuid_) const noexcept {
	::hg::todo_panic();
}

u64 PackageLinker::getArchiveCount() const noexcept {
	return _links.size();
}

s64 PackageLinker::getArchiveSize() const noexcept {
	if (not canLoad()) {
		return -1LL;
	}

	if (_links.empty()) {
		return 0LL;
	}

	// Attention: This assumes that we stored a contiguous byte sequence. Is that actually guaranteed?
	const auto firstByte = _links.front().index.offset;
	const auto lastByte = _links.back().index.offset + _links.back().index.size;

	return lastByte - firstByte;
}

void PackageLinker::remove(mref<Guid> archiveGuid_) {
	::hg::panic();
}

PackageLinker::const_iterator_type PackageLinker::begin() const noexcept {
	return const_iterator_type { *this };
}

PackageLinker::iterator_type PackageLinker::begin() noexcept {
	return iterator_type { *this };
}

PackageLinker::const_iterator_type PackageLinker::cbegin() const noexcept {
	return const_iterator_type { *this };
}

PackageLinker::const_iterator_type PackageLinker::end() const noexcept {
	return const_iterator_type { *this, _links.size() };
}

PackageLinker::iterator_type PackageLinker::end() noexcept {
	return iterator_type { *this, _links.size() };
}

PackageLinker::const_iterator_type PackageLinker::cend() const noexcept {
	return const_iterator_type { *this, _links.size() };
}

uptr<PackageLinker::readonly_iter_archive> PackageLinker::operator[](const_iterator_type where_) const noexcept {

	const auto index = where_->index;
	constexpr auto archiveHeaderSize = sizeof(PackageArchiveHeader);

	// Error: This will only work when there is no compression applied... TBD: Proxy stream compression handling

	// TODO: Get package storage object from provided package accessor.
	// TODO: auto blobAccess = _storageIo->accessReadonlyBlob(Arci<storage::IStorage> {});

	// Warning: Just a prove of concept | TODO: Rework
	auto& package = _package.as<nmpt<storage::AccessPackageReadonly>>()->get().get().fully();
	auto duplicate = package._blob.as<nmpt<storage::AccessBlobReadonly>>()->get().owner().tryAcquireReadonly();

	if (not duplicate.has_value()) {

		return make_uptr<readonly_iter_archive>(
			engine::storage::AccessBlobReadonly {
				std::move(package._blob.as<nmpt<storage::AccessBlobReadonly>>()->get()),
				[this](auto* obj_) {

					::hg::assertrt(obj_ != nullptr);
					auto* const obj = static_cast<ptr<ResourceAccessor<const storage::Io<const resource::Blob>>>>(obj_);

					// Note: After RAII finalization triggers, we need to restore the package's blob accessor
					auto& package = _package.as<nmpt<storage::AccessPackageReadonly>>()->get().get().fully();
					package._blob.as<nmpt<storage::AccessBlobReadonly>>()->get() = std::move(*obj);
				}
			},
			static_cast<streamoff>(index.offset + archiveHeaderSize),
			static_cast<streamsize>(index.size - archiveHeaderSize)
		);
	}

	return make_uptr<readonly_iter_archive>(
		engine::storage::AccessBlobReadonly {
			std::move(duplicate.value()), [](auto*) {}
		},
		static_cast<streamoff>(index.offset + archiveHeaderSize),
		static_cast<streamsize>(index.size - archiveHeaderSize)
	);
}

uptr<PackageLinker::iter_archive> PackageLinker::operator[](iterator_type where_) const noexcept {

	const auto index = where_->index;
	constexpr auto archiveHeaderSize = sizeof(PackageArchiveHeader);

	// Error: This will only work when there is no compression applied... TBD: Proxy stream compression handling

	// TODO: Get package storage object from provided package accessor.
	auto blobAccess = _storageIo->accessReadWriteBlob(Arci<storage::IStorage> {});

	::hg::todo_panic();
	return make_uptr<StorageReadWriteArchive>(
		std::move(blobAccess),
		static_cast<streamoff>(index.offset + archiveHeaderSize),
		static_cast<streamsize>(index.size - archiveHeaderSize)
	);
}

uptr<PackageLinker::readonly_iter_archive> PackageLinker::operator[](cref<LinkedArchive> linked_) const noexcept {

	const auto& index = linked_.index;
	constexpr auto archiveHeaderSize = sizeof(PackageArchiveHeader);

	// Error: This will only work when there is no compression applied... TBD: Proxy stream compression handling

	// TODO: Get package storage object from provided package accessor.
	auto blobAccess = _storageIo->accessReadonlyBlob(Arci<storage::IStorage> {});

	::hg::todo_panic();
}

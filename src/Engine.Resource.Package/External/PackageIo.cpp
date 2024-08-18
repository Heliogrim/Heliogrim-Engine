#include "PackageIo.hpp"

#include <cstring>
#include <Engine.Asserts/Todo.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Collection/AutoArray.hpp>
#include <Engine.Common/Functional/Lambda.hpp>
#include <Engine.Pedantic/Into/Into.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Resource.Archive/StorageReadonlyArchive.hpp>
#include <Engine.Resource.Blob/Blob.hpp>

#include "../Attribute/MagicBytes.hpp"
#include "../Attribute/PackageGuid.hpp"
#include "../Footer/PackageFooter.hpp"
#include "../Header/PackageHeader.hpp"
#include "../Linker/LinkedArchiveIterator.hpp"
#include "Engine.Resource/File.hpp"
#include "Engine.Storage/Url/FileUrl.hpp"

using namespace hg::engine::resource::package;
using namespace hg::engine::resource;
using namespace hg::engine::storage;
using namespace hg;

PackageIo::PackageIo(ref<StorageIo> storageIo_) noexcept :
	_storageIo(std::addressof(storageIo_)) {}

/**/

uptr<Package> PackageIo::create_package_from_storage(ref<AccessBlobReadWrite> blobAccessor_) {

	::hg::assertd(not blobAccessor_.invalid());
	::hg::assertrt(blobAccessor_->readable() && blobAccessor_->writable());

	if (not blobAccessor_->randomReadable() || not blobAccessor_->randomWritable()) {
		// Attention: At the moment we required random access, as we have to jump in the stored memory sequence.
		::hg::todo_panic();
	}

	/**/

	auto package = make_uptr<Package>(
		blobAccessor_,
		PackageHeader {},
		PackageFooter {}
	);

	/**/

	readHeader(*package, streamoff {});

	::hg::assertrt(package->header().magicVersion == PackageMagicVersion[0]);
	::hg::assertrt(std::memcmp(package->header().magicBytes, PackageMagicBytes.data(), PackageMagicBytes.size()) == 0);

	/**/

	const streamoff footerOffset = sizeof(PackageHeader) + package->header().packageSize;
	readFooter(*package, footerOffset);

	// TODO: Validate Package creation ( maybe calc and check crc-32 of footer )
	::hg::assertrt(package->footer().magicVersion == PackageMagicVersion[0]);
	::hg::assertrt(std::memcmp(package->footer().magicBytes, PackageMagicBytes.data(), PackageMagicBytes.size()) == 0);

	/**/

	return package;
}

uptr<Package> PackageIo::create_package_from_storage(ref<AccessBlobReadonly> blobAccessor_) {

	::hg::assertd(not blobAccessor_.invalid());
	::hg::assertrt(blobAccessor_.get().get().readable());

	if (not blobAccessor_.get().get().randomReadable()) {
		// Attention: At the moment we required random access, as we have to jump in the stored memory sequence.
		::hg::todo_panic();
	}

	/**/

	auto package = make_uptr<Package>(
		blobAccessor_,
		PackageHeader {},
		PackageFooter {}
	);

	/**/

	readHeader(*package, streamoff {});

	::hg::assertrt(package->header().magicVersion == PackageMagicVersion[0]);
	::hg::assertrt(std::memcmp(package->header().magicBytes, PackageMagicBytes.data(), PackageMagicBytes.size()) == 0);

	/**/

	const streamoff footerOffset = sizeof(PackageHeader) + package->header().packageSize;
	readFooter(*package, footerOffset);

	// TODO: Validate Package creation ( maybe calc and check crc-32 of footer )
	::hg::assertrt(package->footer().magicVersion == PackageMagicVersion[0]);
	::hg::assertrt(std::memcmp(package->footer().magicBytes, PackageMagicBytes.data(), PackageMagicBytes.size()) == 0);

	/**/

	return package;
}

uptr<Package> PackageIo::create_empty_package(ref<AccessBlobReadWrite> blobAccessor_) {

	::hg::assertd(not blobAccessor_.invalid());
	::hg::assertrt(blobAccessor_->writable() && blobAccessor_->rewritable());

	/**/

	PackageGuid guid {};
	GuidGenerate(guid);

	u8 htonGuid[16] {};
	guid.hton(htonGuid);

	/**/

	return make_uptr<Package>(
		blobAccessor_,
		PackageHeader {
			.magicBytes = { 0x49, 0x4D, 0x50, 0x41, 0x43, 0x4B }/*PackageMagicBytes */,
			.magicVersion = PackageMagicVersion[0],
			.endianness = PackageEndianness::eBigEndian,
			.version = 0x1,
			.guid = {
				START_SUPPRESS_WARNINGS
				htonGuid[0], htonGuid[1], htonGuid[2], htonGuid[3],
				htonGuid[4], htonGuid[5], htonGuid[6], htonGuid[7],
				htonGuid[8], htonGuid[9], htonGuid[10], htonGuid[11],
				htonGuid[12], htonGuid[13], htonGuid[14], htonGuid[15]
				STOP_SUPPRESS_WARNINGS
			},
			.compression = PackageCompression::eNone
		},
		PackageFooter {
			.crc32 = {},
			.magicBytes = { 0x49, 0x4D, 0x50, 0x41, 0x43, 0x4B }/*PackageMagicBytes */,
			.magicVersion = PackageMagicVersion[0],
			.endianess = PackageEndianness::eBigEndian
		}
	);
}

uptr<Package> PackageIo::create_empty_package(ref<AccessBlobReadonly> blobAccessor_) {
	// TODO: Remove this function, as is has no semantic value.
	::hg::panic();
}

ref<Package> PackageIo::readHeader(ref<Package> package_, streamoff offset_) {

	auto& blob = package_._blob;
	::hg::assertd(blob.apply([](const auto& blob_) { return blob_->get().valid() && blob_->get().get().readable(); }));

	/**/

	// TODO: Replace with no-copy version
	BufferArchive arch {};
	arch.resize(sizeof(PackageHeader), _::byte {});
	arch.seek(0);

	blob.apply([&arch, offset_](const auto& blob_) { blob_.get()->get()->read(offset_, arch.getByteSpan()); });

	/**/

	arch.serializeBytes(package_.header().magicBytes, sizeof(package_.header().magicBytes), ArchiveStreamMode::eLoad);
	arch >> package_.header().magicVersion;
	arch >> package_.header().endianness;

	arch >> package_.header().version;
	arch.serializeBytes(package_.header().guid, sizeof(package_.header().guid), ArchiveStreamMode::eLoad);
	arch >> package_.header().compression;

	arch >> package_.header().__reserved__;

	arch >> package_.header().packageSize;
	arch >> package_.header().inflatedSize;

	arch >> package_.header().indexOffset;
	arch >> package_.header().indexSize;
	arch >> package_.header().metaOffset;
	arch >> package_.header().metaSize;
	arch >> package_.header().compDataOffset;
	arch >> package_.header().compDataSize;

	/**/

	return package_;
}

ref<Package> PackageIo::writeHeader(ref<Package> package_, streamoff offset_) {

	auto& blob = package_._blob;
	::hg::assertd(blob.apply([](const auto& blob_) { return blob_->get().valid() && blob_->get().get().writable(); }));

	/**/

	BufferArchive arch {};
	arch.reserve(sizeof(PackageHeader));
	arch.seek(0);

	arch.serializeBytes(package_.header().magicBytes, sizeof(package_.header().magicBytes), ArchiveStreamMode::eStore);
	arch << package_.header().magicVersion;
	arch << package_.header().endianness;

	arch << package_.header().version;
	arch.serializeBytes(package_.header().guid, sizeof(package_.header().guid), ArchiveStreamMode::eStore);
	arch << package_.header().compression;

	arch << package_.header().__reserved__;

	arch << package_.header().packageSize;
	arch << package_.header().inflatedSize;

	arch << package_.header().indexOffset;
	arch << package_.header().indexSize;
	arch << package_.header().metaOffset;
	arch << package_.header().metaSize;
	arch << package_.header().compDataOffset;
	arch << package_.header().compDataSize;

	/**/

	blob.as<nmpt<AccessBlobReadWrite>>().get()->get()->write(offset_, arch.getByteSpan());

	/**/

	return package_;
}

ref<Package> PackageIo::readFooter(ref<Package> package_, streamoff offset_) {

	auto& blob = package_._blob;
	::hg::assertd(blob.apply([](const auto& blob_) { return blob_->get().valid() && blob_->get().get().readable(); }));

	/**/

	// TODO: Replace with no-copy version
	BufferArchive arch {};
	arch.resize(sizeof(PackageFooter), _::byte {});
	arch.seek(0);

	blob.apply([&arch, offset_](const auto& blob_) { blob_.get()->get()->read(offset_, arch.getByteSpan()); });

	/**/

	arch.serializeBytes(package_.footer().crc32, sizeof(package_.footer().crc32), ArchiveStreamMode::eLoad);
	arch.serializeBytes(
		package_.footer().__reserved__,
		sizeof(package_.footer().__reserved__),
		ArchiveStreamMode::eLoad
	);
	arch.serializeBytes(package_.footer().magicBytes, sizeof(package_.footer().magicBytes), ArchiveStreamMode::eLoad);
	arch >> package_.footer().magicVersion;
	arch >> package_.footer().endianess;

	/**/

	return package_;
}

ref<Package> PackageIo::writeFooter(ref<Package> package_, streamoff offset_) {

	auto& blob = package_._blob;
	::hg::assertd(blob.apply([](const auto& blob_) { return blob_->get().valid() && blob_->get().get().writable(); }));

	/**/

	BufferArchive arch {};
	arch.reserve(sizeof(PackageFooter));
	arch.seek(0);

	arch.serializeBytes(package_.footer().crc32, sizeof(package_.footer().crc32), ArchiveStreamMode::eStore);
	arch.serializeBytes(
		package_.footer().__reserved__,
		sizeof(package_.footer().__reserved__),
		ArchiveStreamMode::eStore
	);
	arch.serializeBytes(package_.footer().magicBytes, sizeof(package_.footer().magicBytes), ArchiveStreamMode::eStore);
	arch << package_.footer().magicVersion;
	arch << package_.footer().endianess;

	/**/

	blob.as<nmpt<AccessBlobReadWrite>>().get()->get()->write(offset_, arch.getByteSpan());

	/**/

	return package_;
}

/**/

UniqueValue<PackageLinker> PackageIo::create_linker_from_package(
	ref<AccessPackageReadWrite> packageAccessor_
) {
	auto linker = UniqueValue<PackageLinker> { PackageLinker { *_storageIo, packageAccessor_ } };
	loadLinkerData(*linker);
	return linker;
}

UniqueValue<PackageLinker> PackageIo::create_linker_from_package(
	ref<AccessPackageReadonly> packageAccessor_
) {
	auto linker = UniqueValue<PackageLinker> { PackageLinker { *_storageIo, packageAccessor_ } };
	loadLinkerData(*linker);
	return linker;
}

UniqueValue<PackageLinker> PackageIo::create_empty_linker(
	ref<AccessPackageReadWrite> packageAccessor_
) {
	return UniqueValue<PackageLinker> { PackageLinker { *_storageIo, packageAccessor_ } };
}

/**/

ref<PackageLinker> PackageIo::loadLinkerData(ref<PackageLinker> linker_) {

	::hg::assertrt(linker_.canLoad());

	linker_._package.apply(
		[&linker_](auto accessor_) {

			::hg::assertrt(accessor_->get().valid() && accessor_->get()->header().indexSize > 0uLL);

			if (accessor_->get()->header().compression != PackageCompression::eNone) {
				todo_panic();
			}

			/**/

			const auto& package = (*accessor_)->fully();
			const auto& blob = (*accessor_)->fully()._blob;

			BufferArchive indexArchive {};
			indexArchive.resize(package.header().indexSize);

			/**/

			blob.apply(
				[&indexArchive, &package](auto blob_) {
					blob_->get()->read(
						static_cast<streamoff>(package.header().indexOffset),
						indexArchive.getByteSpan()
					);
				}
			);

			/**/

			const auto estimateArchiveCount = package.header().indexSize / sizeof(PackageIndexEntry);
			const auto indexOffset = package.header().indexOffset;

			for (auto i = 0uLL; i < estimateArchiveCount; ++i) {

				PackageIndexEntry index {};
				indexArchive >> index.offset;
				indexArchive >> index.size;

				/**/

				// throw std::runtime_error("Invalid offset stored at `PackageIndexEntry`.");
				::hg::assertrt(
					index.offset >= static_cast<streamsize>(sizeof(PackageHeader)) && index.offset < indexOffset
				);

				//throw std::runtime_error("Invalid size stored at `PackageIndexEntry`.");
				::hg::assertrt((index.offset + index.size) <= indexOffset);

				/**/

				BufferArchive headerArchive {};
				headerArchive.resize(sizeof(PackageArchiveHeader));

				/**/

				blob.apply(
					[&headerArchive, &index](auto blob_) {
						// Attention: We need to determine whether the offset is in compressed or uncompressed state.
						// Attention: To partially fast load memory, we may need to both offset around.
						// Attention: This is dependent on the compression, and whether the implementation allows random access with the inflated byte offset
						blob_->get()->read(index.offset, headerArchive.getByteSpan());
					}
				);

				/**/

				PackageArchiveHeader header {};
				// Error: we need to use hton / ntoh to prepare guid to get stored correctly over multiple platforms
				headerArchive.serializeBytes(
					std::addressof(header.guid),
					sizeof(header.guid),
					ArchiveStreamMode::eLoad
				);
				headerArchive >> header.type;

				/**/

				linker_._links.emplace_back(std::move(header), std::move(index));
			}

		}
	);
	return linker_;
}

ref<PackageLinker> PackageIo::storeLinkerData(ref<PackageLinker> linker_) {

	assertrt(linker_.canStore());
	// TODO: Deal with LinkerStoreMode::eFlushAndDefrag

	linker_._package.apply(
		Overloaded {
			[]([[maybe_unused]] nmpt<AccessPackageReadonly>) { panic(); },
			[&linker_](nmpt<AccessPackageReadWrite> accessor_) {

				::hg::assertrt(accessor_->get().valid());

				if (accessor_->get()->header().compression != PackageCompression::eNone) {
					todo_panic();
				}

				/**/

				auto& package = (*accessor_)->fully();
				const auto& blob = accessor_->get()->_blob;

				BufferArchive indexArchive {};
				indexArchive.reserve(linker_.getArchiveCount() * sizeof(PackageIndexEntry));

				/**/

				for (const auto& linked : linker_) {
					indexArchive << linked.index.offset;
					indexArchive << linked.index.size;
				}

				/**/

				const streamoff indexOffset = linker_.getArchiveSize() + static_cast<streamoff>(sizeof(PackageHeader));
				blob.as<nmpt<AccessBlobReadWrite>>()->get()->write(indexOffset, indexArchive.getByteSpan());

				/**/

				package.header().indexOffset = indexOffset;
				package.header().indexSize = indexArchive.getByteSpan().size();
			}
		}
	);
	return linker_;
}

/**/

void PackageIo::experimental_full_package_load(
	ref<AccessPackageReadonly> packageAccessor_,
	ref<Vector<uptr<Archive>>> archives_
) {

	auto& package = packageAccessor_->fully();
	if (package.header().compression != PackageCompression::eNone) {
		// TODO: Initialize Decompressor
	}

	/**/

	// TODO: 1. Read Package Header ~ Should be done already while creating the backing object of the accessor.
	// TODO: 2. Read Package Footer ~´Should be done already while creating the backing object of the accessor.

	// TODO: 3. Read compressor data

	AutoArray<u8> compressor {};
	compressor.resize(package.header().compDataSize);

	package._blob.apply(
		[&compressor, &header = package.header()](auto blob_) {
			blob_->get()->read(header.compDataOffset, compressor.getByteSpan());
		}
	);

	// TODO: 4. Read Index Segment

	auto linker = create_linker_from_package(packageAccessor_);
	// Implicitly: loadLinkerData(*linker);

	// TODO: 5. Hydrate Archive Callbacks

	for (const auto& linked : *linker) {
		// Warning: This will detour the shared owning part of the initial package resource
		// Warning:		resulting in a pseudo-locked state, as long as any of the exposed archives is alive.
		archives_.emplace_back(into<Archive>((*linker)[linked]));
	}
}

void PackageIo::experimental_full_package_store(
	ref<AccessPackageReadWrite> packageAccessor_,
	mref<Vector<uptr<Archive>>> archives_
) {

	// TODO: Check whether we still need the meta segment within the package

	auto& package = packageAccessor_->fully();
	if (package.header().compression != PackageCompression::eNone) {
		// TODO: Initialize Compressor
	}

	/**/

	// TODO: 1. Write Package Header

	// Attention: In the first case this is a dummy write to force a stream write down the line ( required for common file io )
	writeHeader(package, streamoff {});

	// TODO: 2. Write compressed linked data

	auto linker = create_empty_linker(packageAccessor_);

	/**/

	for (auto&& archive : archives_) {

		std::span<_::byte> data {};
		if (package.header().compression != PackageCompression::eNone) {
			// TODO: Process linked data with prepare compressor
			data = {};
		} else {}

		linker->store(PackageArchiveHeader {}, std::move(archive));
	}

	// TODO: 3. Write Index Segment

	storeLinkerData(*linker);

	// TODO: 4. Write compressor data

	auto compressorOffset = package.header().indexOffset + package.header().indexSize;
	auto compressorData = std::span<_::byte> {};

	package._blob.as<nmpt<AccessBlobReadWrite>>().get()->get()->write(
		static_cast<streamoff>(compressorOffset),
		clone(compressorData)
	);

	package.header().compDataOffset = compressorOffset;
	package.header().compDataSize = compressorData.size_bytes();

	// TODO: 5. Rewrite Package Header

	const auto packageSize = compressorOffset + compressorData.size_bytes();
	package.header().packageSize = packageSize;

	if (package.header().compression != PackageCompression::eNone) {
		auto compressionRate = 3.2;
		package.header().inflatedSize = static_cast<u64>(
			static_cast<double>(packageSize) / compressionRate
		);
	}

	// Attention: At this point we require the rewrite ability of the underlying storage unit
	writeHeader(package, streamoff {});

	// TODO: 6. Write Package Footer
	writeFooter(package, packageSize);
}

bool PackageIo::isPackageFile(
	cref<storage::FileUrl> fileUrl_,
	cref<Arci<IStorage>> fileStorage_
) {

	const auto extension = static_cast<std::filesystem::path>(fileUrl_.path()).extension();
	if (not extension.empty()) {
		const auto str = extension.string();
		if (not str.ends_with(".imp") && not str.ends_with(".impackage")) {
			return false;
		}
	}

	/**/

	auto blobAccess = _storageIo->accessReadonlyBlob(clone(fileStorage_));
	return isPackageBlob(blobAccess);
}

bool PackageIo::isPackageBlob(ref<AccessBlobReadonly> blob_) {

	PackageHeader header {};

	auto& blob = blob_->fully();
	auto read = blob.read(
		static_cast<streamoff>(offsetof(PackageHeader, magicBytes)),
		std::span { reinterpret_cast<_::byte(&)[6]>(header.magicBytes) }
	);

	// Question: Even though this is deprecated, should we actually check for data integrity and/or matching header <-> footers?
	return read.size() >= PackageMagicBytes.size() && std::memcmp(
		header.magicBytes,
		PackageMagicBytes.data(),
		PackageMagicBytes.size()
	) == 0;
}

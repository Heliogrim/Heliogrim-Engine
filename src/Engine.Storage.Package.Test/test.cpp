#include "pch.h"

/**/

#include <chrono>
#include <cstring>
#include <filesystem>
#include <random>
#include <string>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/UniqueValue.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Resource.Archive/MemoryReadonlyArchive.hpp>
#include <Engine.Resource.Archive/StorageReadWriteArchive.hpp>
#include <Engine.Resource.Blob/Memory/ByteSpanBlob.hpp>
#include <Engine.Resource.Package/ReadonlyPackage.hpp>
#include <Engine.Resource.Package/ReadWritePackage.hpp>
#include <Engine.Resource.Package/Attribute/MagicBytes.hpp>
#include <Engine.Resource.Package/Attribute/PackageGuid.hpp>
#include <Engine.Resource.Package/Header/PackageHeader.hpp>
#include <Engine.Resource.Package/Linker/Indexed.hpp>
#include <Engine.Storage.Package/Factory.hpp>
#include <Engine.Storage.Package/Processor.hpp>
#include <Engine.Storage.Package/Store.hpp>
#include <Engine.Storage.Registry/Registry/StorageRegistry.hpp>
#include <Engine.Storage.Registry/Storage/MemoryStorage.hpp>

using namespace hg::engine::storage::package;
using namespace hg::engine::resource;
using namespace hg;

namespace PackageModule {
	TEST(Package, BaseLifecycle) {

		auto blob = ByteSpanBlob {};
		auto readonlyPackage = engine::storage::package::make_readonly_package(blob);
		auto readWritePackage = engine::storage::package::make_read_write_package(blob);

		constexpr auto nullGuid = Array<_::byte, 16> {};
		EXPECT_NE(readonlyPackage.getHeader().guid, nullGuid);
		EXPECT_NE(readWritePackage.getHeader().guid, nullGuid);
	}

	TEST(Package, StoreEmptyPackageToMemory) {

		// Attention: TBD: Check for system endianness
		auto raw = Array<_::byte, sizeof(PackageHeader) + sizeof(PackageFooter)> {};
		auto memory = std::span<_::byte> { raw };

		/**/

		const PackageVersion version { 27859368927uLL };
		const PackageGuid guid { 278608376uL, 11632uL, 7635uL, 36758197986519uLL };

		/**/

		{
			auto blob = ByteSpanBlob { memory };
			auto package = engine::storage::package::make_read_write_package(blob);

			ref<PackageHeader> header = package.getHeader();
			ref<PackageFooter> footer = package.getFooter();

			header.endianness = PackageEndianness::eLittleEndian;

			header.version = version;
			guid.hton(header.guid);

			header.compression = PackageCompression::eNone;
			header.packageSize = 0uLL;
			header.inflatedSize = 0uLL;
			header.indexOffset = sizeof(PackageHeader);
			header.indexSize = 0uLL;
			header.metaOffset = sizeof(PackageHeader);
			header.metaSize = 0uLL;
			header.compDataOffset = 0uLL;
			header.compDataSize = 0uLL;

			// Write Package data
			EXPECT_TRUE(engine::storage::package::writeLinker(header, package.getLinker(), blob));
			EXPECT_TRUE(engine::storage::package::writeHeader(header, blob));
			EXPECT_TRUE(engine::storage::package::writeFooter(header, footer, blob));
		}

		/**/

		{
			ASSERT_GE(memory.size_bytes(), sizeof(PackageHeader));
			const auto* const storedHeader = reinterpret_cast<const ptr<const PackageHeader>>(memory.data());

			// Check Magic Bytes
			{
				for (auto i = 0; i < PackageMagicBytes.size(); ++i) {
					EXPECT_EQ(storedHeader->magicBytes[i], PackageMagicBytes[i]);
				}

				EXPECT_EQ(storedHeader->magicVersion, PackageMagicVersion[0]);
				EXPECT_EQ(storedHeader->endianness, PackageEndianness::eLittleEndian);
			}

			// Check Package Version
			{
				if (std::endian::native != std::endian::little) {
					auto tmp = std::byteswap(version);
					EXPECT_EQ(storedHeader->version, tmp);
				} else {
					EXPECT_EQ(storedHeader->version, version);
				}
			}

			// Check Package Guid
			{
				auto storedGuid = PackageGuid::ntoh(storedHeader->guid);
				EXPECT_EQ(storedGuid, guid);
			}

			// Check Package Compression
			{
				EXPECT_EQ(storedHeader->compression, PackageCompression::eNone);
			}

			// Check Package/Inflate Size
			{
				EXPECT_EQ(storedHeader->packageSize, 0);
				EXPECT_EQ(storedHeader->inflatedSize, 0);
			}

			// Check Index, Meta and Compression Segment
			{
				if (std::endian::native != std::endian::little) {

					constexpr auto pkgHeadSize = std::byteswap(sizeof(PackageHeader));
					EXPECT_EQ(storedHeader->indexOffset, pkgHeadSize);
					EXPECT_EQ(storedHeader->indexSize, 0);

					EXPECT_EQ(storedHeader->metaOffset, pkgHeadSize);
					EXPECT_EQ(storedHeader->metaSize, 0);

					EXPECT_EQ(storedHeader->compDataOffset, 0);
					EXPECT_EQ(storedHeader->compDataSize, 0);

				} else {

					EXPECT_EQ(storedHeader->indexOffset, sizeof(PackageHeader));
					EXPECT_EQ(storedHeader->indexSize, 0);

					EXPECT_EQ(storedHeader->metaOffset, sizeof(PackageHeader));
					EXPECT_EQ(storedHeader->metaSize, 0);

					EXPECT_EQ(storedHeader->compDataOffset, 0);
					EXPECT_EQ(storedHeader->compDataSize, 0);
				}
			}
		}
	}

	TEST(Package, LoadEmptyPackageFromMemory) {
		// Attention: TBD: Check for system endianess

		auto raw = Array<_::byte, sizeof(PackageHeader) + sizeof(PackageFooter)> {};
		auto memory = std::span<_::byte> { raw.data(), raw.size() };

		/**/

		PackageVersion version { 27859368927uLL };
		PackageGuid guid { 278608376uL, 11632uL, 7635uL, 36758197986519uLL };

		/**/

		{
			Array<_::byte, 16> htonPackageGuid {};
			guid.hton(htonPackageGuid);

			PackageHeader header {
				.magicBytes = PackageMagicBytes,
				.magicVersion = PackageMagicVersion[0],
				.endianness = PackageEndianness::eBigEndian,
				.version = version,
				.guid = htonPackageGuid,
				.compression = PackageCompression::eNone,
				.indexOffset = sizeof(PackageHeader),
				.metaOffset = sizeof(PackageHeader)
			};

			PackageFooter footer {
				.crc32 = {},
				.magicBytes = PackageMagicBytes,
				.magicVersion = PackageMagicVersion[0],
				.endianess = PackageEndianness::eBigEndian
			};

			/**/

			ASSERT_GE(memory.size(), sizeof(header) + sizeof(footer));

			std::memcpy(memory.data(), &header, sizeof(header));
			std::memcpy(memory.data() + sizeof(header), &footer, sizeof(footer));
		}

		/**/

		{
			auto blob = ByteSpanBlob { memory, memory.size() };
			auto package = engine::storage::package::reconstruct_readonly_package(blob);
			const auto& header = package.getHeader();

			// Check Magic Bytes
			{
				for (decltype(PackageMagicBytes)::size_type i = 0; i < PackageMagicBytes.size(); ++i) {
					EXPECT_EQ(_::byte { header.magicBytes[i] }, PackageMagicBytes[i]);
				}

				EXPECT_EQ(header.magicVersion, PackageMagicVersion[0]);
				EXPECT_EQ(header.endianness, PackageEndianness::eBigEndian);
			}

			// Check Package Version
			{
				if (std::endian::native != std::endian::little) {
					auto tmp = std::byteswap(version);
					EXPECT_EQ(header.version, tmp);
				} else {
					EXPECT_EQ(header.version, version);
				}
			}

			// Check Package Guid
			{
				auto cmpGuid = PackageGuid::ntoh(header.guid);
				EXPECT_EQ(cmpGuid, guid);
			}

			// Check Package Compression
			{
				EXPECT_EQ(header.compression, PackageCompression::eNone);
			}

			// Check Package/Inflate Size
			{
				EXPECT_EQ(header.packageSize, 0);
				EXPECT_EQ(header.inflatedSize, 0);
			}

			// Check Index, Meta and Compression Segment
			{
				if (std::endian::native != std::endian::little) {

					constexpr auto pkgHeadSize = std::byteswap(sizeof(PackageHeader));
					EXPECT_EQ(header.indexOffset, pkgHeadSize);
					EXPECT_EQ(header.indexSize, 0);

					EXPECT_EQ(header.metaOffset, pkgHeadSize);
					EXPECT_EQ(header.metaSize, 0);

					EXPECT_EQ(header.compDataOffset, 0);
					EXPECT_EQ(header.compDataSize, 0);

				} else {

					EXPECT_EQ(header.indexOffset, sizeof(PackageHeader));
					EXPECT_EQ(header.indexSize, 0);

					EXPECT_EQ(header.metaOffset, sizeof(PackageHeader));
					EXPECT_EQ(header.metaSize, 0);

					EXPECT_EQ(header.compDataOffset, 0);
					EXPECT_EQ(header.compDataSize, 0);
				}
			}
		}
	}

	/**/

	TEST(PackageLinker, ArchiveDeltaAdd) {

		constexpr auto data = R"(Hello World)"sv;

		auto raw = Array<
			_::byte,
			sizeof(PackageHeader) + sizeof(PackageFooter) +
			sizeof(package::Indexed) * 1uLL + data.size()
		> {};
		auto blob = ByteSpanBlob { raw };

		auto package = make_read_write_package(blob);

		/**/

		auto initialResult = commit_package_changes(package);
		EXPECT_TRUE(initialResult.has_value());

		/**/

		auto guid = ArchiveGuid::random();
		auto archive = make_uptr<BufferArchive>();
		archive->reserve(data.size());

		for (const auto& ch : data) { *archive << ch; }

		/**/

		const auto prevSize = package.getHeader().packageSize;

		auto& linked = package.getLinker().add({ .type = package::PackageArchiveType::eRaw, .guid = guid });
		linked.changes.emplace_back(
			package::ArchiveDeltaAdd {
				streamoff {},
				static_cast<streamsize>(archive->size()),
				*archive
			}
		);

		/**/

		auto updateResult = commit_package_changes(package);
		ASSERT_TRUE(updateResult.has_value());

		const auto expectedSize = prevSize + archive->size() + sizeof(package::Indexed);
		EXPECT_GE(package.getHeader().packageSize, expectedSize);

		EXPECT_EQ(package.getLinker().count(), 1uLL);
		EXPECT_NE(package.getLinker()[0], None);
		EXPECT_NE(package.getLinker()[guid], None);

		/**/

		std::span reintSpan = std::span { raw }.subspan(sizeof(PackageHeader), data.size());
		auto reintArchive = MemoryReadonlyArchive { reintSpan };

		auto recover = string {};
		recover.resize(data.size(), '\0');

		for (auto& ch : recover) {
			reintArchive >> ch;
		}

		EXPECT_EQ(recover, data);
	}

	TEST(PackageLinker, ArchiveDeltaReplace) {

		constexpr auto data0 = R"(This payload should stay at the top.)"sv;
		constexpr auto data1 = R"(This payload should be replaced.)"sv;
		constexpr auto data2 = R"(This payload should stay at the end.)"sv;

		constexpr auto update1 = R"(This payload should replace the previous one)"sv;

		auto raw = Array<
			_::byte,
			sizeof(PackageHeader) + sizeof(PackageFooter) +
			sizeof(package::Indexed) * 3uLL +
			data0.size() + (std::max)(data1.size(), update1.size()) + data2.size()
		> {};
		auto blob = ByteSpanBlob { raw };

		auto package = make_read_write_package(blob);

		/**/

		auto guid0 = ArchiveGuid::random();
		auto guid1 = ArchiveGuid::random();
		auto guid2 = ArchiveGuid::random();

		auto ar0 = make_uptr<BufferArchive>();
		auto ar1 = make_uptr<BufferArchive>();
		auto ar2 = make_uptr<BufferArchive>();

		ar0->reserve(data0.size());
		ar1->reserve(data1.size());
		ar2->reserve(data2.size());

		for (const auto& ch : data0) { *ar0 << ch; }
		for (const auto& ch : data1) { *ar1 << ch; }
		for (const auto& ch : data2) { *ar2 << ch; }

		auto& linker = package.getLinker();

		auto& lk0 = linker.add({ .type = package::PackageArchiveType::eRaw, .guid = guid0 });
		auto& lk1 = linker.add({ .type = package::PackageArchiveType::eRaw, .guid = guid1 });
		auto& lk2 = linker.add({ .type = package::PackageArchiveType::eRaw, .guid = guid2 });

		lk0.changes.emplace_back(package::ArchiveDeltaAdd { .where = 0uLL, .size = static_cast<streamsize>(ar0->size()), .data = *ar0 });
		lk1.changes.emplace_back(package::ArchiveDeltaAdd { .where = 0uLL, .size = static_cast<streamsize>(ar1->size()), .data = *ar1 });
		lk2.changes.emplace_back(package::ArchiveDeltaAdd { .where = 0uLL, .size = static_cast<streamsize>(ar2->size()), .data = *ar2 });

		ASSERT_TRUE(commit_package_changes(package).has_value());

		/**/

		const auto prevSize = package.getHeader().packageSize;

		auto target = package.getLinker()[guid1];
		ASSERT_NE(target, None);

		auto archive = make_uptr<BufferArchive>();
		archive->reserve(update1.size());
		for (const auto& ch : update1) { *archive << ch; }

		target->changes.emplace_back(
			package::ArchiveDeltaReplace {
				streamoff {}, static_cast<streamsize>(archive->size()), *archive
			}
		);
		EXPECT_EQ(target->changes.size(), 1uLL);

		auto updateResult = commit_package_changes(package);
		ASSERT_TRUE(updateResult.has_value());

		// Warning: We just require/assume that update1.size is bigger than data1.size ( unsigned type )
		constexpr auto sizeDiff = update1.size() - data1.size();
		const auto expectedSize = prevSize + sizeDiff;
		EXPECT_LE(package.getHeader().packageSize, expectedSize);

		/**/

		EXPECT_EQ(package.getLinker().count(), 3uLL);
		EXPECT_NE(package.getLinker()[guid1], None);
		EXPECT_EQ(package.getLinker()[guid0]->data.size, ar0->size());
		EXPECT_EQ(package.getLinker()[guid1]->data.size, archive->size());
		EXPECT_EQ(package.getLinker()[guid2]->data.size, ar2->size());
	}

	TEST(PackageLinker, ArchiveDeltaDrop) {

		constexpr auto data0 = R"(This payload should stay at the top.)"sv;
		constexpr auto data1 = R"(This payload should be removed.)"sv;
		constexpr auto data2 = R"(This payload should stay at the end.)"sv;

		auto raw = Array<
			_::byte,
			sizeof(PackageHeader) + sizeof(PackageFooter) +
			sizeof(package::Indexed) * 3uLL +
			data0.size() + data1.size() + data2.size()
		> {};
		auto blob = ByteSpanBlob { raw };

		auto package = make_read_write_package(blob);

		/**/

		auto guid0 = ArchiveGuid::random();
		auto guid1 = ArchiveGuid::random();
		auto guid2 = ArchiveGuid::random();

		auto ar0 = make_uptr<BufferArchive>();
		auto ar1 = make_uptr<BufferArchive>();
		auto ar2 = make_uptr<BufferArchive>();

		ar0->reserve(data0.size());
		ar1->reserve(data1.size());
		ar2->reserve(data2.size());

		for (const auto& ch : data0) { *ar0 << ch; }
		for (const auto& ch : data1) { *ar1 << ch; }
		for (const auto& ch : data2) { *ar2 << ch; }

		auto& linker = package.getLinker();

		auto& lk0 = linker.add({ .type = package::PackageArchiveType::eRaw, .guid = guid0 });
		auto& lk1 = linker.add({ .type = package::PackageArchiveType::eRaw, .guid = guid1 });
		auto& lk2 = linker.add({ .type = package::PackageArchiveType::eRaw, .guid = guid2 });

		lk0.changes.emplace_back(package::ArchiveDeltaAdd { .where = 0uLL, .size = static_cast<streamsize>(ar0->size()), .data = *ar0 });
		lk1.changes.emplace_back(package::ArchiveDeltaAdd { .where = 0uLL, .size = static_cast<streamsize>(ar1->size()), .data = *ar1 });
		lk2.changes.emplace_back(package::ArchiveDeltaAdd { .where = 0uLL, .size = static_cast<streamsize>(ar2->size()), .data = *ar2 });

		ASSERT_TRUE(commit_package_changes(package).has_value());

		/**/

		const auto prevSize = package.getHeader().packageSize;

		auto target = package.getLinker()[guid1];
		ASSERT_NE(target, None);

		target->changes.emplace_back(package::ArchiveDeltaDrop { streamoff {}, static_cast<streamsize>(ar1->size()) });
		EXPECT_EQ(target->changes.size(), 1uLL);

		auto updateResult = commit_package_changes(package);
		ASSERT_TRUE(updateResult.has_value());

		const auto expectedSize = (prevSize - ar1->size()) - sizeof(package::Indexed);
		EXPECT_EQ(package.getHeader().packageSize, expectedSize);

		/**/

		EXPECT_EQ(package.getLinker().count(), 2uLL);
		EXPECT_EQ(package.getLinker()[guid0]->data.size, ar0->size());
		EXPECT_EQ(package.getLinker()[guid1], None);
		EXPECT_EQ(package.getLinker()[guid2]->data.size, ar2->size());

		const auto expectedBlobSize = sizeof(PackageHeader) + expectedSize + sizeof(PackageFooter);
		EXPECT_EQ(blob.size(), expectedBlobSize);
	}

	// TODO: TEST(PackageLinker, MultiArchiveDelta) {
	// TODO: 	ASSERT_FALSE(true);
	// TODO: }
}

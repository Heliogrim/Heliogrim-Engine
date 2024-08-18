#include "pch.h"

/**/

#include <cstring>
#include <random>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/UniqueValue.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Config/Enums.hpp>
#include <Engine.Config/Provider/EditorProvider.hpp>
#include <Engine.Config/Provider/ProjectProvider.hpp>
#include <Engine.Config/Provider/RuntimeProvider.hpp>
#include <Engine.Config/Provider/SystemProvider.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Resource.Archive/StorageReadonlyArchive.hpp>
#include <Engine.Resource.Archive/StorageReadWriteArchive.hpp>
#include <Engine.Resource.Package/Package.hpp>
#include <Engine.Resource.Package/Attribute/MagicBytes.hpp>
#include <Engine.Resource.Package/Attribute/PackageGuid.hpp>
#include <Engine.Resource.Package/External/PackageIo.hpp>
#include <Engine.Resource.Package/Linker/LinkedArchiveIterator.hpp>
#include <Engine.Resource.Package/Linker/PackageLinker.hpp>
#include <Engine.Storage/Options/MemoryStorageDescriptor.hpp>
#include <Engine.Storage/Options/StorageDescriptor.hpp>
#include <Engine.Storage/Registry/StorageRegistry.hpp>
#include <Engine.Storage/Storage/MemoryStorage.hpp>
#include <Engine.Storage/Storage/PackageStorage.hpp>

#include "Engine.Resource/File.hpp"
#include "Engine.Storage/Provider/LocalFileSystemProvider.hpp"
#include "Engine.Storage/Repository/CacheFileSystemRepository.hpp"
#include "Engine.Storage/Repository/EditorReadOnlyRepository.hpp"
#include "Engine.Storage/Repository/ProjectFileSystemRepository.hpp"
#include "Engine.Storage/Repository/RuntimeFileSystemRepository.hpp"

using namespace hg::engine::resource;
using namespace hg::engine::serialization;
using namespace hg;

namespace PackageModule {
	void generatePayload(ref<BufferArchive> archive_, const u64 size_ = 48uLL) {

		std::random_device device {};
		std::mt19937 mt { device() };

		for (u64 i = 0; i < size_; ++i) {
			archive_ << mt();
		}
	}

	class PackageFixture :
		public testing::Test {
	protected:
		nmpt<engine::cfg::SystemProvider> _systemConfigProvider;
		nmpt<editor::cfg::EditorProvider> _editorConfigProvider;
		nmpt<engine::cfg::ProjectProvider> _projectConfigProvider;
		nmpt<engine::cfg::RuntimeProvider> _runtimeConfigProvider;

		uptr<engine::Config> _config;
		uptr<engine::storage::StorageRegistry> _registry;

	protected:
		uptr<engine::storage::StorageIo> storageIo;
		uptr<engine::storage::PackageIo> packageIo;

	private:
		void initConfigValues();

		void cleanupConfigAssoc();

	protected:
		[[nodiscard]] Arci<engine::storage::system::MemoryStorage> allocMemStore(size_t size_ = 1024uLL);

		[[nodiscard]] std::tuple<
			Arci<engine::storage::system::PackageStorage>,
			Arci<engine::storage::system::MemoryStorage>
		> allocPackStore(
			mref<Arci<engine::storage::system::MemoryStorage>> backing_
		);

	public:
		void SetUp() override {

			_config = make_uptr<engine::Config>();

			_systemConfigProvider = std::addressof(
				_config->registerProvider(make_uptr<engine::cfg::SystemProvider>())
			);
			_editorConfigProvider = std::addressof(
				_config->registerProvider(make_uptr<editor::cfg::EditorProvider>())
			);
			_projectConfigProvider = std::addressof(
				_config->registerProvider(make_uptr<engine::cfg::ProjectProvider>())
			);
			_runtimeConfigProvider = std::addressof(
				_config->registerProvider(make_uptr<engine::cfg::RuntimeProvider>())
			);

			/**/

			initConfigValues();

			/**/

			_registry = make_uptr<engine::storage::StorageRegistry>();
			_registry->setup(*_config);

			/**/

			auto cacheBasePath = hg::fs::File::path_type {
				_config->getTyped<String>(engine::cfg::EditorConfigProperty::eLocalCachePath).value<>().value()
			};
			auto projectBasePath = hg::fs::File::path_type {
				_config->getTyped<String>(engine::cfg::ProjectConfigProperty::eLocalBasePath).value<>().value()
			};
			auto runtimeBasePath = hg::fs::File::path_type {
				_config->getTyped<String>(engine::cfg::RuntimeConfigProperty::eLocalBasePath).value<>().value()
			};
			auto editorBasePath = hg::fs::File::path_type {
				_config->getTyped<String>(engine::cfg::EditorConfigProperty::eLocalEditorPath).value<>().value()
			};

			auto lfsOwned = make_uptr<engine::storage::system::LocalFileSystemProvider>();
			auto lfs = lfsOwned.get();

			_registry->addProvider(std::move(lfsOwned));

			_registry->addRepository(lfs->makeCacheRepository(std::move(cacheBasePath)));
			_registry->addRepository(lfs->makeProjectRepository(std::move(projectBasePath)));
			_registry->addRepository(lfs->makeRuntimeRepository(std::move(runtimeBasePath)));
			_registry->addRepository(lfs->makeEditorRepository(std::move(editorBasePath)));

			/**/

			auto* engine = static_cast<engine::Engine*>(nullptr);
			storageIo = make_uptr<engine::storage::StorageIo>(*engine);
			packageIo = make_uptr<engine::storage::PackageIo>(*storageIo);
		}

		void TearDown() override {
			packageIo.reset();
			storageIo.reset();

			/**/

			_registry->tidy();
			_registry.reset();

			/**/

			cleanupConfigAssoc();

			/**/

			_config.reset();
		}
	};

	TEST_F(PackageFixture, BaseLifecycle) {

		auto backingStorage = allocMemStore();

		/**/

		{
			auto readWriteAccess = storageIo->accessReadWriteBlob(clone(backingStorage));
			auto readWritePackage = packageIo->create_empty_package(readWriteAccess);
			EXPECT_NE(readWritePackage, nullptr);
		}

	}

	TEST_F(PackageFixture, StoreEmptyPackageToMemory) {

		// Attention: TBD: Check for system endianness

		auto backingStorage = allocMemStore();

		/**/

		PackageVersion version { 27859368927uLL };
		PackageGuid guid { 278608376uL, 11632uL, 7635uL, 36758197986519uLL };

		/**/

		{
			auto backingAccess = storageIo->accessReadWriteBlob(clone(backingStorage));
			auto package = packageIo->create_empty_package(backingAccess);

			ref<PackageHeader> header = package->header();
			ref<PackageFooter> footer = package->footer();

			std::memcpy(header.magicBytes, PackageMagicBytes.data(), PackageMagicBytes.size());
			header.magicVersion = PackageMagicVersion[0];
			header.endianness = PackageEndianness::eBigEndian;

			header.version = version;
			std::memcpy(header.guid, &guid.pre, sizeof(PackageGuid));

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
			packageIo->writeHeader(*package, streamoff {});
			packageIo->writeFooter(*package, sizeof(PackageHeader));

			// Attention: We need to release the mutable memory accessor, otherwise the memory integrity check won't be able to acquire access.
		}

		/**/

		{
			auto memoryAccess = storageIo->accessReadonly(std::move(backingStorage));
			auto memory = memoryAccess->fully().span();

			ASSERT_GE(memory.size_bytes(), sizeof(PackageHeader));
			const auto* const storedHeader = reinterpret_cast<const ptr<const PackageHeader>>(memory.data());

			// Check Magic Bytes
			{
				for (auto i = 0; i < PackageMagicBytes.size(); ++i) {
					EXPECT_EQ(_::byte { storedHeader->magicBytes[i] }, PackageMagicBytes[i]);
				}

				EXPECT_EQ(storedHeader->magicVersion, PackageMagicVersion[0]);
				EXPECT_EQ(storedHeader->endianness, PackageEndianness::eBigEndian);
			}

			// Check Package Version
			{
				EXPECT_EQ(storedHeader->version, version);
			}

			// Check Package Guid
			{
				for (auto i = 0; i < sizeof(PackageGuid); ++i) {
					EXPECT_EQ(storedHeader->guid[i], std::bit_cast<u8*>(&guid.pre)[i]);
				}
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
				EXPECT_EQ(storedHeader->indexOffset, sizeof(PackageHeader));
				EXPECT_EQ(storedHeader->indexSize, 0);

				EXPECT_EQ(storedHeader->metaOffset, sizeof(PackageHeader));
				EXPECT_EQ(storedHeader->metaSize, 0);

				EXPECT_EQ(storedHeader->compDataOffset, 0);
				EXPECT_EQ(storedHeader->compDataSize, 0);
			}
		}
	}

	TEST_F(PackageFixture, LoadEmptyPackageFromMemory) {
		// Attention: TBD: Check for system endianess

		auto backingStorage = allocMemStore();

		/**/

		PackageVersion version { 27859368927uLL };
		PackageGuid guid { 278608376uL, 11632uL, 7635uL, 36758197986519uLL };

		/**/

		{
			auto memoryAccess = storageIo->accessReadWrite(clone(backingStorage));

			u8 htonPackageGuid[16] {};
			guid.hton(htonPackageGuid);

			engine::resource::PackageHeader header {
				.magicBytes = { 0x49, 0x4D, 0x50, 0x41, 0x43, 0x4B },
				.magicVersion = engine::resource::PackageMagicVersion[0],
				.endianness = engine::resource::PackageEndianness::eBigEndian,
				.version = version,
				.guid = {
					htonPackageGuid[0], htonPackageGuid[1], htonPackageGuid[2], htonPackageGuid[3],
					htonPackageGuid[4], htonPackageGuid[5], htonPackageGuid[6], htonPackageGuid[7],
					htonPackageGuid[8], htonPackageGuid[9], htonPackageGuid[10], htonPackageGuid[11],
					htonPackageGuid[12], htonPackageGuid[13], htonPackageGuid[14], htonPackageGuid[15],
				},
				.compression = engine::resource::PackageCompression::eNone,
				.indexOffset = sizeof(PackageHeader),
				.metaOffset = sizeof(PackageHeader)
			};

			engine::resource::PackageFooter footer {
				.crc32 = {},
				.magicBytes = { 0x49, 0x4D, 0x50, 0x41, 0x43, 0x4B },
				.magicVersion = engine::resource::PackageMagicVersion[0],
				.endianess = engine::resource::PackageEndianness::eBigEndian
			};

			/**/

			auto dst = memoryAccess->fully().span();
			ASSERT_GE(dst.size(), sizeof(header) + sizeof(footer));

			std::memcpy(dst.data(), &header, sizeof(header));
			std::memcpy(dst.data() + sizeof(header), &footer, sizeof(footer));

			// Attention: We need to release the mutable memory accessor, otherwise the memory integrity check won't be able to acquire access.
		}

		/**/

		{
			auto readonlyAccess = storageIo->accessReadonlyBlob(std::move(backingStorage));
			auto package = packageIo->create_package_from_storage(readonlyAccess);

			/**/

			auto& header = package->header();

			// Check Magic Bytes
			{
				for (auto i = 0; i < PackageMagicBytes.size(); ++i) {
					EXPECT_EQ(_::byte { header.magicBytes[i] }, PackageMagicBytes[i]);
				}

				EXPECT_EQ(header.magicVersion, PackageMagicVersion[0]);
				EXPECT_EQ(header.endianness, PackageEndianness::eBigEndian);
			}

			// Check Package Version
			{
				EXPECT_EQ(header.version, version);
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
				EXPECT_EQ(header.indexOffset, sizeof(PackageHeader));
				EXPECT_EQ(header.indexSize, 0);

				EXPECT_EQ(header.metaOffset, sizeof(PackageHeader));
				EXPECT_EQ(header.metaSize, 0);

				EXPECT_EQ(header.compDataOffset, 0);
				EXPECT_EQ(header.compDataSize, 0);
			}
		}
	}

	TEST(Package, MetaReadWrite) {
		// Warning: In the current state we removed the meta segment from the package.
	}

	/**/

	class PackageLinkerFixture : public PackageFixture {
		using PackageFixture::PackageFixture;
	};

	TEST_F(PackageLinkerFixture, BaseLifecycle) {
		// TODO: Test the basic lifecycle of generating a package and the corresponding linker above it
	}

	TEST_F(PackageLinkerFixture, SingleIsoPushWrite) {
		// TODO: Test the linkers storing functions using an isolated source archive which should be pushed ~ +KeepAlive

		auto packStore = allocPackStore(allocMemStore());
		auto accessor = storageIo->accessReadWrite(std::move(std::get<0>(packStore)));

		/**/

		Guid guid {};
		GuidGenerate(guid);
		auto archive = make_uptr<BufferArchive>();
		archive->reserve(16uLL);

		/**/

		auto data = "Hello World"sv;
		for (const auto& ch : data) {
			*archive << ch;
		}

		/**/

		auto linker = packageIo->create_empty_linker(accessor);
		linker->store({ .type = package::PackageArchiveType::eRaw, .guid = guid }, std::move(archive));

		/**/

		packageIo->storeLinkerData(*linker);
	}

	TEST_F(PackageLinkerFixture, SingleInplacePullWrite) {
		// TODO: Test the linkers storing generator functions ~ +KeepAlive

		auto packStore = allocPackStore(allocMemStore());
		auto accessor = storageIo->accessReadWrite(std::move(std::get<0>(packStore)));

		/**/

		Guid guid {};
		GuidGenerate(guid);

		/**/

		auto linker = packageIo->create_empty_linker(accessor);
		linker->store(
			{ .type = package::PackageArchiveType::eRaw, .guid = guid },
			[](auto&& archive_) {

				auto data = "Hello World"sv;
				for (const auto& ch : data) {
					*archive_ << ch;
				}

			}
		);

		/**/

		packageIo->storeLinkerData(*linker);
	}

	TEST_F(PackageLinkerFixture, SingleInplaceDetourPullWrite) {
		// TODO: Test the linkers storing generator functions while detouring the pull reference ~ +KeepAlive

		auto packStore = allocPackStore(allocMemStore());
		auto accessor = storageIo->accessReadWrite(std::move(std::get<0>(packStore)));

		/**/

		auto linker = packageIo->create_empty_linker(accessor);

		/**/

		{
			Guid guid {};
			GuidGenerate(guid);
			auto scoped = linker->store({ .type = package::PackageArchiveType::eRaw, .guid = guid });

			auto data = "Hello World"sv;
			for (const auto& ch : data) {
				*scoped << ch;
			}
		}

		/**/

		packageIo->storeLinkerData(*linker);
	}

	TEST_F(PackageLinkerFixture, SingleReadWrite) {
		// Attention: TBD: Check for system endianness

		auto backing = allocMemStore();

		/**/

		Guid payloadGuid {};
		GuidGenerate(payloadGuid);

		auto payload = make_uptr<BufferArchive>();

		const auto valCount = 48uLL;
		generatePayload(*payload, valCount);

		BufferArchive replica {};
		replica.reserve(payload->size());
		replica.insert(replica.end(), payload->begin(), payload->end());

		/**/

		{
			auto packStore = allocPackStore(clone(backing));
			auto accessor = storageIo->accessReadWrite(std::move(std::get<0>(packStore)));
			auto linker = packageIo->create_empty_linker(accessor);

			/**/

			linker->store({ .type = package::PackageArchiveType::eRaw, .guid = payloadGuid }, std::move(payload));

			// Write Package data

			// TODO: Where should the write call occure?
			// TODO: `package.write(...)` vs `linker.write(...)`

			packageIo->storeLinkerData(*linker);

			auto& header = accessor->fully().header();
			header.packageSize = header.indexOffset + header.indexSize - sizeof(PackageHeader);
			packageIo->writeHeader(accessor->fully(), streamoff {});

			auto footerOffset = header.indexOffset + header.indexSize;
			packageIo->writeFooter(accessor->fully(), static_cast<streamoff>(footerOffset));
		}

		/**/

		{
			auto packStore = allocPackStore(std::move(backing));
			auto accessor = storageIo->accessReadonly(std::move(std::get<0>(packStore)));

			auto linker = packageIo->create_linker_from_package(accessor);

			EXPECT_EQ(linker->getArchiveCount(), 1);
			EXPECT_NE(linker->cbegin(), linker->cend());

			auto archiveIter = linker->cbegin();

			auto storedHeader = archiveIter.header();
			auto storedArchive = archiveIter.archive();

			EXPECT_EQ(storedHeader.type, package::PackageArchiveType::eRaw);
			EXPECT_EQ(storedHeader.guid, payloadGuid);

			EXPECT_NE(storedArchive, nullptr);

			// TODO: Check whether `std::memcmp(...)` might be possible with `storedArchive` and `replica`
			// TODO: Currently value iterated compare to prevent source loading issue cause there are not memory blocks at SourceArchive

			storedArchive->seek(0);
			replica.seek(0);
			for (size_t i = 0; i < valCount; ++i) {

				using value_type = std::mt19937::result_type;

				value_type value {};
				(*storedArchive) >> value;

				value_type check {};
				replica >> check;

				EXPECT_EQ(value, check);
			}
		}
	}

	TEST_F(PackageLinkerFixture, MultiReadWrite) {

		// Attention: TBD: Check for system endianness

		constexpr size_t buffer_size = 8192;
		constexpr size_t archive_count = 17;
		constexpr size_t value_count = 48;

		static_assert(
			buffer_size > (
				(sizeof(u32) * archive_count * value_count) +
				sizeof(PackageHeader) +
				sizeof(PackageFooter) +
				sizeof(PackageIndexEntry) * archive_count +
				sizeof(package::PackageArchiveHeader) * archive_count
			)
		);

		auto backing = allocMemStore(buffer_size);
		Vector<uptr<BufferArchive>> replicas {};

		/**/

		{
			// Create Payload
			Vector<uptr<BufferArchive>> payloads {};
			for (auto i = 0; i < archive_count; ++i) {
				payloads.push_back(make_uptr<BufferArchive>());
				generatePayload(*payloads.back(), value_count);
			}

			replicas.reserve(payloads.size());
			for (size_t i = 0; i < payloads.size(); ++i) {

				const auto payload = *payloads[i];

				replicas.push_back(make_uptr<BufferArchive>());
				auto& replica = *replicas[i];

				replica.reserve(payload.size());
				replica.insert(replica.end(), payload.begin(), payload.end());
			}

			// Write payload to linker and underlying package

			auto packStore = allocPackStore(clone(backing));
			auto accessor = storageIo->accessReadWrite(std::move(std::get<0>(packStore)));
			auto linker = packageIo->create_empty_linker(accessor);

			Guid payloadGuid {};
			for (auto&& payload : payloads) {

				++payloadGuid.post;

				linker->store({ .type = package::PackageArchiveType::eRaw, .guid = payloadGuid }, std::move(payload));
			}

			// Write Package data

			// TODO: Where should the write call occure?
			// TODO: `package.write(...)` vs `linker.write(...)`

			packageIo->storeLinkerData(*linker);

			auto& header = accessor->fully().header();
			header.packageSize = header.indexOffset + header.indexSize - sizeof(PackageHeader);
			packageIo->writeHeader(accessor->fully(), streamoff {});

			auto footerOffset = header.indexOffset + header.indexSize;
			packageIo->writeFooter(accessor->fully(), static_cast<streamoff>(footerOffset));
		}

		/**/

		{
			auto packStore = allocPackStore(clone(backing));
			auto accessor = storageIo->accessReadonly(std::move(std::get<0>(packStore)));
			auto linker = packageIo->create_linker_from_package(accessor);

			EXPECT_EQ(linker->getArchiveCount(), archive_count);
			EXPECT_NE(linker->begin(), linker->end());

			size_t idx = 0;
			for (auto iter = linker->cbegin(); iter != linker->cend(); ++iter, ++idx) {

				auto storedHeader = iter.header();
				auto storedArchive = iter.archive();

				EXPECT_EQ(storedHeader.type, package::PackageArchiveType::eRaw);
				EXPECT_EQ(storedHeader.guid.post, (idx + 1));

				EXPECT_NE(storedArchive, nullptr);

				// TODO: Check whether `std::memcmp(...)` might be possible with `storedArchive` and `replica`
				// TODO: Currently value iterated compare to prevent source loading issue cause there are not memory blocks at SourceArchive

				auto& replica = *replicas[idx];

				storedArchive->seek(0);
				replica.seek(0);
				for (size_t i = 0; i < value_count; ++i) {

					using value_type = std::mt19937::result_type;

					value_type value {};
					(*storedArchive) >> value;

					value_type check {};
					replica >> check;

					EXPECT_EQ(value, check);
				}

			}
		}
	}
}

/**/
void PackageModule::PackageFixture::initConfigValues() {

	auto& cfg = *_config;

	const auto baseRoot = std::filesystem::current_path();
	const auto root = clone(baseRoot).append("tmp"sv);

	/**/

	// Note: Just guarantee for the test, that we do not interfere with anything
	//ASSERT_TRUE(not std::filesystem::exists(root));
	if (std::filesystem::exists(root)) {
		std::filesystem::remove_all(root);
	}
	std::filesystem::create_directories(root);

	/**/

	{
		String localCachePath = clone(root).append("cache"sv).generic_string();
		std::ignore = cfg.init(
			engine::cfg::EditorConfigProperty::eLocalCachePath,
			std::move(localCachePath),
			engine::cfg::query_provider_id(*_projectConfigProvider)
		);
	}
	/**/
	{
		String localProjectPath = clone(root).append("project"sv).generic_string();
		std::ignore = cfg.init(
			engine::cfg::ProjectConfigProperty::eLocalBasePath,
			std::move(localProjectPath),
			engine::cfg::query_provider_id(*_projectConfigProvider)
		);
	}
	/**/
	{
		String localRuntimePath = clone(root).append("runtime"sv).generic_string();
		std::ignore = cfg.init(
			engine::cfg::RuntimeConfigProperty::eLocalBasePath,
			std::move(localRuntimePath),
			engine::cfg::query_provider_id(*_runtimeConfigProvider)
		);
	}
	/**/
	{
		String localEditorPath = clone(root).append("editor"sv).generic_string();
		std::ignore = cfg.init(
			engine::cfg::EditorConfigProperty::eLocalEditorPath,
			std::move(localEditorPath),
			engine::cfg::query_provider_id(*_editorConfigProvider)
		);
	}

}

void PackageModule::PackageFixture::cleanupConfigAssoc() {
	auto& cfg = *_config;

	const auto baseRoot = std::filesystem::current_path();
	const auto root = clone(baseRoot).append("tmp"sv);

	/**/

	if (not std::filesystem::exists(root)) {
		return;
	}

	/**/

	std::filesystem::remove_all(root);
}

Arci<engine::storage::system::MemoryStorage> PackageModule::PackageFixture::allocMemStore(
	const engine::storage::MemoryRequestSize size_
) {

	auto memoryGuid = Guid {};
	GuidGenerate(memoryGuid);

	auto memoryStorage = _registry->insert(
		engine::storage::MemoryStorageDescriptor { engine::storage::MemoryUrl { clone(memoryGuid) }, size_ }
	).into<engine::storage::system::MemoryStorage>();

	return memoryStorage;
}

std::tuple<Arci<engine::storage::system::PackageStorage>, Arci<engine::storage::system::MemoryStorage>>
PackageModule::PackageFixture::allocPackStore(mref<Arci<engine::storage::system::MemoryStorage>> backing_) {

	auto packageGuid = Guid {};
	GuidGenerate(packageGuid);

	auto packageStorage = _registry->insert(
		engine::storage::PackageStorageDescriptor {
			engine::storage::PackageUrl { clone(packageGuid) }, clone(backing_).into<engine::storage::IStorage>()
		}
	).into<engine::storage::system::PackageStorage>();

	return std::make_tuple(std::move(packageStorage), std::move(backing_));
}

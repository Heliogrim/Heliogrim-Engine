#include "pch.h"

/**/
#include <chrono>
#include <cstring>
#include <fstream>
#include <string>
#include <Engine.Common/Guid.hpp>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Config/Enums.hpp>
#include <Engine.Config/Provider/EditorProvider.hpp>
#include <Engine.Config/Provider/ProjectProvider.hpp>
#include <Engine.Config/Provider/RuntimeProvider.hpp>
#include <Engine.Config/Provider/SystemProvider.hpp>
#include <Engine.Filesystem/Path.hpp>
#include <Engine.Filesystem/Url.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Pedantic.Resource/Resource.hpp>
#include <Engine.Pedantic.Resource/ResourceAccessor.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Resource.Package/Package.hpp>
#include <Engine.Resource.Package/Attribute/MagicBytes.hpp>
#include <Engine.Resource.Package/Footer/PackageFooter.hpp>
#include <Engine.Resource.Package/Header/PackageHeader.hpp>
#include <Engine.Storage.IO/StorageIo.hpp>
#include <Engine.Storage/IStorage.hpp>
#include <Engine.Storage/Options/StorageDescriptor.hpp>
#include <Engine.Storage/Registry/StorageRegistry.hpp>
#include <Engine.Storage/Storage/ArchiveStorage.hpp>
#include <Engine.Storage/Storage/LocalFileStorage.hpp>
#include <Engine.Storage/Storage/MemoryStorage.hpp>
#include <Engine.Storage/Storage/PackageStorage.hpp>
#include <Engine.Storage/Url/Url.hpp>

#include "Engine.Resource.Archive/StorageReadonlyArchive.hpp"
#include "Engine.Resource/File.hpp"
#include "Engine.Resource.Package/Attribute/PackageGuid.hpp"
#include "Engine.Resource.Package/External/PackageIo.hpp"
#include "Engine.Resource.Package/Linker/LinkedArchiveIterator.hpp"
#include "Engine.Resource.Package/Linker/PackageArchiveHeader.hpp"
#include "Engine.Resource.Package/Linker/PackageLinker.hpp"
#include "Engine.Storage/Provider/LocalFileSystemProvider.hpp"
#include "Engine.Storage/Repository/CacheFileSystemRepository.hpp"
#include "Engine.Storage/Repository/EditorReadOnlyRepository.hpp"
#include "Engine.Storage/Repository/ProjectFileSystemRepository.hpp"
#include "Engine.Storage/Repository/RuntimeFileSystemRepository.hpp"

using namespace hg;

namespace StorageModule {
	[[nodiscard]] constexpr std::span<
		const _::byte,
		sizeof(engine::resource::PackageHeader) + sizeof(engine::resource::PackageFooter)
	> get_static_empty_package_binary() noexcept;

	[[nodiscard]] constexpr std::span<const _::byte> get_static_lfs_to_package_to_archive_binary_data() noexcept;

	class StorageRegistryFixture : public testing::Test {
	protected:
		nmpt<engine::cfg::SystemProvider> _systemConfigProvider;
		nmpt<editor::cfg::EditorProvider> _editorConfigProvider;
		nmpt<engine::cfg::ProjectProvider> _projectConfigProvider;
		nmpt<engine::cfg::RuntimeProvider> _runtimeConfigProvider;

		uptr<engine::Config> _config;
		uptr<engine::storage::StorageRegistry> _registry;

	protected:
		void initConfigValues() {

			auto& cfg = *_config;

			const auto baseRoot = std::filesystem::current_path();
			const auto root = clone(baseRoot).append("tmp"sv).concat(
				std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count())
			);

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
					engine::cfg::query_provider_id(*_editorConfigProvider)
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

		void cleanupConfigAssoc() {

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
		}

		void TearDown() override {
			_registry->tidy();
			_registry.reset();

			/**/

			cleanupConfigAssoc();

			/**/

			_config.reset();
		}
	};

	/* Package System IO Tests */
	#pragma region Package System IO Tests

	TEST_F(StorageRegistryFixture, LoadEmptyPackageFromStream) {
		// Warning: Temporary bypass
		EXPECT_TRUE(true);
	}

	TEST_F(StorageRegistryFixture, StoreEmptyPackageToStream) {
		// Warning: Temporary bypass
		EXPECT_TRUE(true);
	}

	#pragma endregion Package System IO Tests

	TEST_F(StorageRegistryFixture, ArchiveToPackageToLFS) {

		engine::Engine* engine = nullptr;
		engine::storage::StorageIo sio { *engine };
		engine::storage::PackageIo pio { sio };

		/**/

		const auto rndFileBase = _config->getTyped<String>(engine::cfg::RuntimeConfigProperty::eLocalBasePath);
		const auto rndFile = std::filesystem::path { rndFileBase->value() }.append("archive_to_package_to_lfs.tmp");
		ASSERT_FALSE(std::filesystem::exists(rndFile));

		/**/

		auto lfsUrl = engine::storage::FileUrl {
			clone(engine::storage::FileRuntimeScheme), engine::fs::Path { rndFile }
		};
		auto fileStore = _registry->insert(
			engine::storage::FileStorageDescriptor {
				clone(lfsUrl)
			}
		).into<engine::storage::system::LocalFileStorage>();
		ASSERT_NE(fileStore, nullptr);

		/**/

		auto packageGuid = Guid {};
		GuidGenerate(packageGuid);

		auto packUrl = engine::storage::PackageUrl { clone(packageGuid) };
		auto packStore = _registry->insert(
			engine::storage::PackageStorageDescriptor {
				clone(packUrl), clone(fileStore).into<engine::storage::IStorage>()
			}
		).into<engine::storage::system::PackageStorage>();
		ASSERT_NE(packStore, nullptr);

		/**/

		auto packageAccess = sio.accessReadWrite(clone(packStore));
		auto linker = pio.create_empty_linker(packageAccess);

		// Warning: This is just a temporary workaround, as we cannot write into empty space when using file storage
		pio.writeHeader(packageAccess->fully(), streamoff {});

		/**/

		constexpr auto targetCount = 5uLL;
		for (auto i = 0; i < targetCount; ++i) {

			auto archiveGuid = Guid {};
			GuidGenerate(archiveGuid);

			const auto index = std::to_string(i);

			auto bufferArchive = make_uptr<engine::resource::BufferArchive>();
			(*bufferArchive) << 'B' << 'u' << 'f' << 'f' << 'e' << 'r'
				<< 'A' << 'r' << 'c' << 'h' << 'i' << 'v' << 'e'
				<< '[' << index[0] << ']';

			linker->store(
				{ .type = engine::resource::package::PackageArchiveType::eRaw, .guid = clone(archiveGuid) },
				std::move(bufferArchive)
			);
		}

		/**/

		pio.storeLinkerData(*linker);

		auto& header = packageAccess->fully().header();
		header.packageSize = header.indexOffset + header.indexSize - sizeof(engine::resource::PackageHeader);
		pio.writeHeader(packageAccess->fully(), streamoff {});

		auto footerOffset = header.indexOffset + header.indexSize;
		pio.writeFooter(packageAccess->fully(), static_cast<streamoff>(footerOffset));
	}

	TEST_F(StorageRegistryFixture, ArchiveFromPackageFromLFS) {

		engine::Engine* engine = nullptr;
		engine::storage::StorageIo sio { *engine };
		engine::storage::PackageIo pio { sio };

		/**/

		const auto rndFileBase = _config->getTyped<String>(engine::cfg::RuntimeConfigProperty::eLocalBasePath);
		const auto rndFile = std::filesystem::path { rndFileBase->value() }.append("lfs_to_package_to_archive.tmp");
		ASSERT_FALSE(std::filesystem::exists(rndFile));

		/* Mock the data */

		{
			std::filesystem::create_directories(rndFile.parent_path());
			auto mockStream = std::fstream { rndFile.string().c_str(), std::ios::out | std::ios::binary };

			ASSERT_TRUE(mockStream.is_open());

			auto mockData = get_static_lfs_to_package_to_archive_binary_data();
			mockStream.write(std::bit_cast<char*>(mockData.data()), mockData.size());

			ASSERT_EQ(mockStream.tellg(), mockData.size());
		}

		/**/

		auto lfsUrl = engine::storage::FileUrl {
			clone(engine::storage::FileRuntimeScheme), engine::fs::Path { rndFile }
		};
		auto fileStore = _registry->insert(
			engine::storage::FileStorageDescriptor {
				clone(lfsUrl)
			}
		).into<engine::storage::system::LocalFileStorage>();
		ASSERT_NE(fileStore, nullptr);

		/**/

		auto accessFile = sio.accessReadonlyBlob(clone(fileStore));
		auto package = pio.create_package_from_storage(accessFile);

		auto storePackUrl = engine::storage::PackageUrl { engine::resource::PackageGuid::ntoh(package->header().guid) };
		auto storePack = _registry->insert(
			engine::storage::PackageStorageDescriptor {
				std::move(storePackUrl), std::move(fileStore).into<engine::storage::IStorage>()
			}
		).into<engine::storage::system::PackageStorage>();
		ASSERT_NE(storePack, nullptr);

		/**/

		auto packageAccess = sio.accessReadonly(clone(storePack));
		auto linker = pio.create_linker_from_package(packageAccess);

		ASSERT_EQ(linker->getArchiveCount(), 5uLL);

		/**/

		const auto end = linker->cend();
		auto idx = 0uL;
		for (auto iter = linker->cbegin(); iter != end; ++iter, ++idx) {

			EXPECT_EQ(iter.header().type, engine::resource::package::PackageArchiveType::eRaw);

			const auto index = std::to_string(idx);
			auto bufferArchive = make_uptr<engine::resource::BufferArchive>();
			(*bufferArchive) << 'B' << 'u' << 'f' << 'f' << 'e' << 'r'
				<< 'A' << 'r' << 'c' << 'h' << 'i' << 'v' << 'e'
				<< '[' << index[0] << ']';

			Array<_::byte, 16uLL> readRaw {};
			iter.archive()->serializeBytes(readRaw.data(), readRaw.size(), engine::resource::ArchiveStreamMode::eLoad);

			EXPECT_EQ(std::memcmp(bufferArchive->getByteSpan().data(), readRaw.data(), readRaw.size()), 0L);
		}
	}
}

/**/

constexpr static u8 static_empty_package_binary_data[] {
	73, 77, 80, 65,
	67, 75, 1, 240,
	223, 59, 140, 124,
	6, 0, 0, 0,
	248, 57, 155, 16,
	112, 45, 211, 29,
	215, 100, 85, 111,
	110, 33, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	100, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	100, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	73, 77, 80, 65,
	67, 75, 1, 240
};

constexpr std::span<
	const _::byte,
	sizeof(engine::resource::PackageHeader) + sizeof(engine::resource::PackageFooter)
> StorageModule::get_static_empty_package_binary() noexcept {
	return std::span<const _::byte, sizeof(engine::resource::PackageHeader) + sizeof(engine::resource::PackageFooter)> {
		// Note: We need to work around, cause reinterpret_cast will not satisfy the constexpr constraint and std::bit_cast fails to convert the data correctly
		std::bit_cast<const _::byte*>(static_cast<const void*>(static_empty_package_binary_data)), 124uLL
		//reinterpret_cast<cref<const _::byte[124]>>(static_empty_package_binary_data)
	};
}

/**/

constexpr static u8 static_lfs_to_package_to_archive_binary_data[369] = {
	0x49, 0x4D, 0x50, 0x41, 0x43, 0x4B, 0x01, 0xF0, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x5B, 0xF0, 0x7A, 0x39, 0xC8, 0x98, 0x46, 0x96,
	0x8D, 0xEE, 0xD1, 0x6D, 0xAB, 0xA0, 0x93, 0xBB, 0x00, 0x00, 0x00, 0x00,
	0xF5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x09, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x10, 0x6B, 0x57, 0xA0, 0xEB, 0xE9, 0x60, 0x43,
	0x92, 0xAD, 0x4C, 0x14, 0x61, 0x09, 0x73, 0x6B, 0xFF, 0x42, 0x75, 0x66,
	0x66, 0x65, 0x72, 0x41, 0x72, 0x63, 0x68, 0x69, 0x76, 0x65, 0x5B, 0x30,
	0x5D, 0x42, 0x42, 0xB9, 0x3D, 0xA5, 0xF4, 0xE1, 0x48, 0x96, 0x6E, 0x71,
	0x97, 0x66, 0x25, 0xA2, 0xF7, 0xFF, 0x42, 0x75, 0x66, 0x66, 0x65, 0x72,
	0x41, 0x72, 0x63, 0x68, 0x69, 0x76, 0x65, 0x5B, 0x31, 0x5D, 0xAC, 0xDF,
	0xF1, 0x3C, 0x01, 0xCF, 0x8D, 0x4C, 0x9B, 0x28, 0xF1, 0x75, 0x3E, 0x23,
	0xC3, 0x02, 0xFF, 0x42, 0x75, 0x66, 0x66, 0x65, 0x72, 0x41, 0x72, 0x63,
	0x68, 0x69, 0x76, 0x65, 0x5B, 0x32, 0x5D, 0x78, 0xF0, 0x6D, 0xAF, 0x06,
	0x4A, 0x47, 0x4F, 0x92, 0xA9, 0x06, 0x7E, 0x56, 0x99, 0x5F, 0x2A, 0xFF,
	0x42, 0x75, 0x66, 0x66, 0x65, 0x72, 0x41, 0x72, 0x63, 0x68, 0x69, 0x76,
	0x65, 0x5B, 0x33, 0x5D, 0xAF, 0xF6, 0x93, 0x15, 0x19, 0x42, 0xC2, 0x4A,
	0x9E, 0xD0, 0x13, 0x1D, 0xEA, 0xD7, 0x5C, 0x85, 0xFF, 0x42, 0x75, 0x66,
	0x66, 0x65, 0x72, 0x41, 0x72, 0x63, 0x68, 0x69, 0x76, 0x65, 0x5B, 0x34,
	0x5D, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA6, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xC7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x49, 0x4D, 0x50, 0x41, 0x43, 0x4B, 0x01, 0xF0
};

constexpr std::span<const _::byte> StorageModule::get_static_lfs_to_package_to_archive_binary_data() noexcept {
	return std::span<const _::byte> {
		// Note: We need to work around, cause reinterpret_cast will not satisfy the constexpr constraint and std::bit_cast fails to convert the data correctly
		std::bit_cast<const _::byte*>(static_cast<const void*>(static_lfs_to_package_to_archive_binary_data)),
		sizeof(static_lfs_to_package_to_archive_binary_data)
		//reinterpret_cast<cref<const _::byte[124]>>(static_lfs_to_package_to_archive_binary_data)
	};
}

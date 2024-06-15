#include "pch.h"

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
#include <Engine.Storage/IStorage.hpp>
#include <Engine.Storage/Options/StorageDescriptor.hpp>
#include <Engine.Storage/Registry/StorageRegistry.hpp>
#include <Engine.Storage/Storage/ArchiveStorage.hpp>
#include <Engine.Storage/Storage/MemoryStorage.hpp>
#include <Engine.Storage/Storage/PackageStorage.hpp>
#include <Engine.Storage/Url/Url.hpp>
#include <Engine.Storage.IO/StorageIo.hpp>

#include <Engine.Resource.Package/Package/MagicBytes.hpp>
#include <Engine.Resource.Package/Package/Package.hpp>
#include <Engine.Resource.Package/Package/PackageFooter.hpp>
#include <Engine.Resource.Package/Package/PackageHeader.hpp>

using namespace hg;

namespace StorageModule {
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

	TEST_F(StorageRegistryFixture, PackageLoadFromMemory) {

		constexpr size_t memorySize = 1024uLL;
		auto memoryGuid = Guid {};
		GuidGenerate(memoryGuid);

		auto memoryStorage = _registry->insert(
			engine::storage::MemoryStorageDescriptor { engine::storage::MemoryUrl { clone(memoryGuid) }, memorySize }
		).into<engine::storage::system::MemoryStorage>();

		/**/

		engine::Engine* engine = nullptr;
		engine::storage::StorageIo sio { *engine };

		/**/

		{
			auto memoryAccess = sio.accessReadWrite(clone(memoryStorage));

			Guid packageGuid;
			GuidGenerate(packageGuid);

			u8 htonPackageGuid[16] {};
			packageGuid.hton(htonPackageGuid);

			engine::resource::PackageHeader header {
				.magicBytes = { 0x49, 0x4D, 0x50, 0x41, 0x43, 0x4B },
				.magicVersion = engine::resource::PackageMagicVersion[0],
				.endianness = engine::resource::PackageEndianness::eBigEndian,
				.version = 0x1,
				.guid = {
					htonPackageGuid[0], htonPackageGuid[1], htonPackageGuid[2], htonPackageGuid[3],
					htonPackageGuid[4], htonPackageGuid[5], htonPackageGuid[6], htonPackageGuid[7],
					htonPackageGuid[8], htonPackageGuid[9], htonPackageGuid[10], htonPackageGuid[11],
					htonPackageGuid[12], htonPackageGuid[13], htonPackageGuid[14], htonPackageGuid[15],
				},
				.compression = engine::resource::PackageCompression::eNone
			};

			engine::resource::PackageFooter footer {
				.crc32 = {},
				.magicBytes = { 0x49, 0x4D, 0x50, 0x41, 0x43, 0x4B },
				.magicVersion = engine::resource::PackageMagicVersion[0],
				.endianess = engine::resource::PackageEndianness::eBigEndian
			};

			/**/

			char* dst {};
			memcpy(dst, &header, sizeof(header));
			memcpy(dst + sizeof(header), &footer, sizeof(footer));
		}

		/**/

		// Error: We don't know the package guid before we decode the anonymous backing storage
		auto packageGuid = Guid {};
		GuidGenerate(packageGuid);

		auto packageStorage = _registry->insert(
			engine::storage::PackageStorageDescriptor {
				engine::storage::PackageUrl { clone(packageGuid) },
				std::move(memoryStorage).into<engine::storage::IStorage>()
			}
		).into<engine::storage::system::PackageStorage>();

		/**/

		auto packageAccess = sio.accessReadonly(std::move(packageStorage));
		const auto& storedHeader = packageAccess->header();
		const auto& storedFooter = packageAccess->footer();
		const auto* const linker = packageAccess->getLinker();

		EXPECT_EQ(storedHeader.packageSize, 0);
		EXPECT_EQ(storedHeader.inflatedSize, 0);
		EXPECT_EQ(storedFooter.crc32[0], 0);
	}
	#pragma endregion Package System IO Tests
}

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
#include <Engine.Storage/Url/Url.hpp>

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

	/* Storage Registry Tests */

	TEST_F(StorageRegistryFixture, BaseLifecycle) {}

	/* Filesystem Repository Tests */
	#pragma region Filesystem Repository Tests

	TEST_F(StorageRegistryFixture, CacheBaseLifecycle) {

		const auto query = engine::storage::FileUrl { "file+cache"sv, engine::fs::Path { "cached.txt"sv } };

		/**/

		ASSERT_FALSE(_registry->hasStorage(clone(query)));

		/**/

		auto descriptor = engine::storage::FileStorageDescriptor { clone(query) };
		ASSERT_TRUE(_registry->insert(std::move(descriptor)) != nullptr);
		ASSERT_TRUE(_registry->hasStorage(clone(query)));

		/**/

		ASSERT_TRUE(_registry->removeStorageByUrl(clone(query)) != nullptr);
		ASSERT_FALSE(_registry->hasStorage(clone(query)));
	}

	TEST_F(StorageRegistryFixture, ProjectBaseLifecycle) {

		const auto query = engine::storage::FileUrl { "file+project"sv, engine::fs::Path { "project.txt"sv } };

		/**/

		ASSERT_FALSE(_registry->hasStorage(clone(query)));

		/**/

		auto descriptor = engine::storage::FileStorageDescriptor { clone(query) };
		ASSERT_TRUE(_registry->insert(std::move(descriptor)) != nullptr);
		ASSERT_TRUE(_registry->hasStorage(clone(query)));

		/**/

		ASSERT_TRUE(_registry->removeStorageByUrl(clone(query)) != nullptr);
		ASSERT_FALSE(_registry->hasStorage(clone(query)));
	}

	TEST_F(StorageRegistryFixture, RuntimeBaseLifecycle) {

		const auto query = engine::storage::FileUrl { "file+runtime"sv, engine::fs::Path { "runtime.txt"sv } };

		/**/

		ASSERT_FALSE(_registry->hasStorage(clone(query)));

		/**/

		auto descriptor = engine::storage::FileStorageDescriptor { clone(query) };
		ASSERT_TRUE(_registry->insert(std::move(descriptor)) != nullptr);
		ASSERT_TRUE(_registry->hasStorage(clone(query)));

		/**/

		ASSERT_TRUE(_registry->removeStorageByUrl(clone(query)) != nullptr);
		ASSERT_FALSE(_registry->hasStorage(clone(query)));
	}

	TEST_F(StorageRegistryFixture, EditorBaseLifecycle) {

		const auto query = engine::storage::FileUrl { "file+editor"sv, engine::fs::Path { "editor.txt"sv } };

		/**/

		ASSERT_FALSE(_registry->hasStorage(clone(query)));

		/**/

		auto descriptor = engine::storage::FileStorageDescriptor { clone(query) };
		ASSERT_TRUE(_registry->insert(std::move(descriptor)) != nullptr);
		ASSERT_TRUE(_registry->hasStorage(clone(query)));

		/**/

		ASSERT_TRUE(_registry->removeStorageByUrl(clone(query)) != nullptr);
		ASSERT_FALSE(_registry->hasStorage(clone(query)));
	}

	#pragma endregion Filesystem Repository Tests

	/* Memory Repository Tests */
	#pragma region Memory Repository Tests

	TEST_F(StorageRegistryFixture, MemoryBaseLifecycle) {

		auto guid = Guid {};
		GuidGenerate(guid);

		const auto query = engine::storage::MemoryUrl { std::move(guid) };
		constexpr auto requestSize = engine::storage::MemoryRequestSize { 32uLL };
		constexpr auto properties = engine::storage::MemoryProperty::eUndefined;

		/**/

		ASSERT_FALSE(_registry->hasStorage(clone(query)));

		/**/

		auto descriptor = engine::storage::MemoryStorageDescriptor { clone(query), requestSize, properties };
		ASSERT_TRUE(_registry->insert(std::move(descriptor)) != nullptr);
		ASSERT_TRUE(_registry->hasStorage(clone(query)));

		/**/

		ASSERT_TRUE(_registry->removeStorageByUrl(clone(query)) != nullptr);
		ASSERT_FALSE(_registry->hasStorage(clone(query)));
	}

	#pragma endregion Memory Repository Tests

	/* Packagesystem Repository Tests */
	#pragma region Packagesystem Repository Tests

	TEST_F(StorageRegistryFixture, ArchiveBaseMemoryLifecycle) {

		auto memoryGuid = Guid {};
		GuidGenerate(memoryGuid);

		auto memoryStorage = _registry->insert(
			engine::storage::MemoryStorageDescriptor {
				engine::storage::MemoryUrl { clone(memoryGuid) }
			}
		);

		/**/

		auto archiveGuid = Guid {};
		GuidGenerate(archiveGuid);

		ASSERT_FALSE(_registry->hasStorage(engine::storage::ArchiveUrl { clone(archiveGuid) }));

		/**/

		auto descriptor = engine::storage::ArchiveStorageDescriptor {};
		ASSERT_TRUE(_registry->insert(std::move(descriptor)) != nullptr);
		ASSERT_TRUE(_registry->hasStorage(engine::storage::ArchiveUrl { clone(archiveGuid) }));

		/**/

		ASSERT_TRUE(_registry->removeStorageByUrl(engine::storage::ArchiveUrl { clone(archiveGuid) }) != nullptr);
		ASSERT_FALSE(_registry->hasStorage(engine::storage::ArchiveUrl { clone(archiveGuid) }));
	}

	TEST_F(StorageRegistryFixture, PackageMountLifecycle) {}

	TEST_F(StorageRegistryFixture, PackageContentLifecycle) {}

	#pragma endregion Packagesystem Repository Tests
}

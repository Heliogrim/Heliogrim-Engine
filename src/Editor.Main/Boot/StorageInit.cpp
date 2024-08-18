#include "StorageInit.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Config/Enums.hpp>
#include <Engine.Storage/IStorageRegistry.hpp>
#include <Engine.Storage/StorageModule.hpp>
#include <Engine.Storage/Provider/LocalFileSystemProvider.hpp>
#include <Engine.Storage/Repository/CacheFileSystemRepository.hpp>
#include <Engine.Storage/Repository/EditorReadOnlyRepository.hpp>
#include <Engine.Storage/Repository/ProjectFileSystemRepository.hpp>

using namespace hg;

void editor::boot::initStorage(cref<engine::Config> cfg_, ref<engine::StorageModule> module_) {

	auto lfsOwned = make_uptr<engine::storage::system::LocalFileSystemProvider>();
	auto lfs = lfsOwned.get();

	/**/

	auto& registry = *module_.getRegistry();
	registry.addProvider(std::move(lfsOwned));

	/**/

	{
		auto cachePath = cfg_.getTyped<String>(engine::cfg::ProjectConfigProperty::eLocalCachePath);
		::hg::assertrt(cachePath.has_value() && cachePath.value().has_value());

		registry.addRepository(lfs->makeCacheRepository(fs::Path { cachePath->value() }));
	}

	/**/

	{
		auto projectPath = cfg_.getTyped<String>(engine::cfg::ProjectConfigProperty::eLocalBasePath);
		::hg::assertrt(projectPath.has_value() && projectPath.value().has_value());

		registry.addRepository(lfs->makeProjectRepository(fs::Path { projectPath->value() }));
	}

	/**/

	{
		auto cachePath = cfg_.getTyped<String>(engine::cfg::EditorConfigProperty::eLocalCachePath);
		::hg::assertrt(cachePath.has_value() && cachePath.value().has_value());

		registry.addRepository(lfs->makeCacheRepository(fs::Path { cachePath->value() }));
	}

	/**/

	{
		auto editorPath = cfg_.getTyped<String>(engine::cfg::EditorConfigProperty::eLocalEditorPath);
		::hg::assertrt(editorPath.has_value() && editorPath.value().has_value());

		registry.addRepository(lfs->makeEditorRepository(fs::Path { editorPath->value() }));
	}
}

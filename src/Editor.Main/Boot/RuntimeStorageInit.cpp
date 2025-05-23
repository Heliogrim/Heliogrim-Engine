#include "RuntimeStorageInit.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Config/Enums.hpp>
#include <Engine.Storage.Registry/IStorageRegistry.hpp>
#include <Engine.Storage/StorageModule.hpp>
#include <Engine.Storage.Registry/Provider/LocalFileSystemProvider.hpp>
#include <Engine.Storage.Registry/Repository/CacheFileSystemRepository.hpp>
#include <Engine.Storage.Registry/Repository/RuntimeFileSystemRepository.hpp>

using namespace hg;

void editor::boot::initRuntimeStorage(cref<engine::Config> cfg_, ref<engine::StorageModule> module_) {

	// Problem: We need to retrieve the previous created lfs provider from the Engine::init storage function
	auto lfsOwned = make_uptr<engine::storage::system::LocalFileSystemProvider>();
	auto lfs = lfsOwned.get();

	/**/

	auto& registry = *module_.getRegistry();
	registry.addProvider(std::move(lfsOwned));

	/**/

	{
		auto cachePath = cfg_.getTyped<String>(engine::cfg::RuntimeConfigProperty::eLocalCachePath);
		::hg::assertrt(cachePath.has_value() && cachePath.value().has_value());

		registry.addRepository(lfs->makeCacheRepository(fs::Path { cachePath->value() }));
	}

	/**/

	{
		auto runtimePath = cfg_.getTyped<String>(engine::cfg::RuntimeConfigProperty::eLocalBasePath);
		::hg::assertrt(runtimePath.has_value() && runtimePath.value().has_value());

		registry.addRepository(lfs->makeRuntimeRepository(fs::Path { runtimePath->value() }));
	}
}

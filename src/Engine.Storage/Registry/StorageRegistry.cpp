#include "StorageRegistry.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Config/Enums.hpp>
#include <Engine.Env/Check.hpp>
#include <Engine.Storage/Url/Url.hpp>

#include "../IStorage.hpp"
#include "../IStorageProvider.hpp"
#include "../IStorageRepository.hpp"
#include "../Options/IStorageDescriptor.hpp"
#include "../Options/StorageDescriptor.hpp"
#include "../Provider/LocalFileSystemProvider.hpp"
#include "../Provider/MemoryProvider.hpp"
#include "../Provider/PackageSystemProvider.hpp"
#include "../Repository/ArchiveSystemRepository.hpp"
#include "../Repository/CacheFileSystemRepository.hpp"
#include "../Repository/EditorReadOnlyRepository.hpp"
#include "../Repository/MemoryRepository.hpp"
#include "../Repository/PackageSystemRepository.hpp"
#include "../Repository/ProjectFileSystemRepository.hpp"
#include "../Repository/RuntimeFileSystemRepository.hpp"

using namespace hg::engine::storage;
using namespace hg;

StorageRegistry::StorageRegistry() noexcept :
	IStorageRegistry(),
	_mtx(),
	_provider(),
	_repositories() {}

StorageRegistry::~StorageRegistry() noexcept = default;

void StorageRegistry::setup(cref<Config> config_) {

	// TODO: Get meta information
	constexpr auto isEditor = env::check<env::EnvProps::eIsEditor>();
	constexpr auto isShipment = env::check<env::EnvProps::eIsShipment>();
	constexpr auto isEditorShipment = isEditor and isShipment;

	/**/

	// TODO: Set by project settings or editor installation
	auto cacheBasePath = hg::fs::File::path_type {
		config_.getTyped<String>(cfg::EditorConfigProperty::eLocalCachePath).value<>().value()
	};
	// TODO: Set by project settings
	auto projectBasePath = hg::fs::File::path_type {
		config_.getTyped<String>(cfg::ProjectConfigProperty::eLocalBasePath).value<>().value()
	};
	// TODO: Set by build-process and shipment
	auto runtimeBasePath = hg::fs::File::path_type {
		config_.getTyped<String>(cfg::RuntimeConfigProperty::eLocalBasePath).value<>().value()
	};
	// TODO: Set by project settings or editor installation
	auto editorBasePath = hg::fs::File::path_type {
		config_.getTyped<String>(cfg::EditorConfigProperty::eLocalEditorPath).value<>().value()
	};

	/**/

	auto* const basePmrAllocator = std::pmr::new_delete_resource();
	auto pmrAlloc = make_uptr<std::pmr::unsynchronized_pool_resource>(basePmrAllocator);

	auto memOwned = make_uptr<system::MemoryProvider>(std::move(pmrAlloc));
	auto mem = memOwned.get();

	auto lfsOwned = make_uptr<system::LocalFileSystemProvider>();
	auto lfs = lfsOwned.get();

	auto pkgOwner = make_uptr<system::PackageSystemProvider>(*this);
	auto pkg = pkgOwner.get();

	/**/

	addProvider(std::move(memOwned));
	addProvider(std::move(lfsOwned));
	addProvider(std::move(pkgOwner));
	// TODO (Optional) : addProvider(make_uptr<FtpSystemProvider>());
	// TODO (Optional) : addProvider(make_uptr<SourceVersionControlSystemProvider>());
	// TODO (Optional) : -> addProvider(make_uptr<PerforceSystemProvider>())

	/**/

	addRepository(mem->makeMemoryRepository());

	addRepository(pkg->makeArchiveRepository());
	addRepository(pkg->makePackageRepository());

	addRepository(lfs->makeCacheRepository(std::move(cacheBasePath)));
	addRepository(lfs->makeProjectRepository(std::move(projectBasePath)));
	addRepository(lfs->makeRuntimeRepository(std::move(runtimeBasePath)));
	addRepository(lfs->makeEditorRepository(std::move(editorBasePath)));
}

void StorageRegistry::tidy() {

	_SCTRL_GATE(_mtx);

	/**/

	_repositories.clear();
	_provider.clear();
}

void StorageRegistry::addProvider(mref<uptr<system::IStorageProvider>> provider_) {
	_SCTRL_GATE(_mtx);
	_provider.emplace_back(std::move(provider_));
}

bool StorageRegistry::removeProvider(mref<std::function<bool(nmpt<system::IStorageProvider> provider_)>> selector_) {
	::hg::todo_panic();
}

void StorageRegistry::addRepository(nmpt<system::IStorageRepository> repository_) {

	_SCTRL_GATE(_mtx);

	const auto providers = repository_->getProviders();
	for (const auto& required : providers) {
		// Verify that this registry has ownership of the required providers
		::hg::assertrt(
			std::ranges::find(
				_provider,
				required.get(),
				[](const auto& owned_) {
					return owned_.get();
				}
			) == std::ranges::end(_provider)
		);
	}

	_repositories.emplace_back(std::move(repository_));
}

bool StorageRegistry::removeRepository(
	mref<std::function<bool(ref<system::IStorageRepository> repository_)>> selector_
) {
	::hg::todo_panic();
}

bool StorageRegistry::hasStorage(mref<Url> url_) const noexcept {

	const auto scheme = url_.scheme();
	for (const auto& repository : _repositories) {
		const auto candidates = repository->getUrlScheme();
		for (const auto& candidate : candidates) {
			if (candidate == scheme && repository->hasStorage(url_)) {
				return true;
			}
			break;
		}
	}

	return false;
}

Arci<IStorage> StorageRegistry::getStorageByUrl(mref<Url> url_) const {

	const auto scheme = url_.scheme();
	for (const auto& repository : _repositories) {

		const auto candidates = repository->getUrlScheme();
		for (const auto& candidate : candidates) {
			if (candidate != scheme) {
				continue;
			}

			if (auto obj = repository->getStorageByUrl(url_); obj != nullptr) {
				return obj;
			}
			break;
		}
	}

	::hg::panic();
}

Arci<IStorage> StorageRegistry::findStorageByUrl(mref<Url> url_) const noexcept {

	const auto scheme = url_.scheme();
	for (const auto& repository : _repositories) {

		const auto candidates = repository->getUrlScheme();
		for (const auto& candidate : candidates) {
			if (candidate != scheme) {
				continue;
			}

			if (auto obj = repository->getStorageByUrl(url_); obj != nullptr) {
				return obj;
			}
			break;
		}
	}

	return {};
}

Arci<IStorage> StorageRegistry::insert(mref<StorageDescriptor> descriptor_) {
	::hg::assertrt(descriptor_.valid());

	// Warning: Temporary naive implementation
	const auto scheme = descriptor_.targetScheme();
	for (const auto& repository : _repositories) {

		const auto candidates = repository->getUrlScheme();
		for (const auto& candidate : candidates) {
			if (candidate == scheme) {
				return repository->createStorage(std::move(descriptor_));
			}
			break;
		}
	}

	return {};
}

Arci<IStorage> StorageRegistry::removeStorageByUrl(mref<Url> url_) {

	const auto scheme = url_.scheme();
	for (const auto& repository : _repositories) {

		const auto candidates = repository->getUrlScheme();
		for (const auto& candidate : candidates) {
			if (candidate != scheme) {
				continue;
			}

			auto obj = repository->getStorageByUrl(url_);
			if (obj != nullptr && repository->removeStorage(obj.get())) {
				return obj;
			}
			break;
		}
	}

	return {};
}

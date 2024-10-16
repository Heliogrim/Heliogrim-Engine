#include "StorageRegistry.hpp"

#include <algorithm>
#include <ranges>
#include <Engine.Asserts/Todo.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Config/Enums.hpp>
#include <Engine.Env/Check.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Storage.Registry/Url/Url.hpp>

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

	auto* const basePmrAllocator = std::pmr::new_delete_resource();
	auto pmrAlloc = make_uptr<std::pmr::unsynchronized_pool_resource>(basePmrAllocator);

	auto memOwned = make_uptr<system::MemoryProvider>(std::move(pmrAlloc));
	auto mem = memOwned.get();

	auto pkgOwner = make_uptr<system::PackageSystemProvider>(*this);
	auto pkg = pkgOwner.get();

	/**/

	addProvider(std::move(memOwned));
	addProvider(std::move(pkgOwner));
	// TODO (Optional) : addProvider(make_uptr<FtpSystemProvider>());
	// TODO (Optional) : addProvider(make_uptr<SourceVersionControlSystemProvider>());
	// TODO (Optional) : -> addProvider(make_uptr<PerforceSystemProvider>())

	/**/

	addRepository(mem->makeMemoryRepository());

	addRepository(pkg->makeArchiveRepository());
	addRepository(pkg->makePackageRepository());
}

void StorageRegistry::tidy() {

	_SCTRL_GATE(_mtx);

	/**/

	_repositories.clear();

	std::ranges::for_each(
		std::ranges::reverse_view(_provider),
		[](auto& provider_) {
			provider_.reset();
		}
	);
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
	for (auto required : providers) {
		// Verify that this registry has ownership of the required providers
		::hg::assertrt(
			std::ranges::find(
				_provider,
				required.get(),
				[](const auto& owned_) {
					return owned_.get();
				}
			) != std::ranges::end(_provider)
		);
	}

	/**/

	const auto found = std::ranges::find(_repositories, repository_);
	if (found != _repositories.end()) {
		IM_DEBUG_LOGF(
			"Tried to add repository `{:x}` to storage registry which already exists.",
			reinterpret_cast<intptr_t>(repository_.get())
		);
		return;
	}

	/**/

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

bool StorageRegistry::findReferrerStorages(
	mref<UrlScheme> scheme_,
	mref<Arci<IStorage>> ref_,
	ref<Vector<Arci<IStorage>>> collector_
) const {

	const auto subject = ::hg::move(ref_);

	for (const auto& repository : _repositories) {

		const auto candidates = repository->getUrlScheme();
		for (const auto& candidate : candidates) {
			if (candidate != scheme_) {
				continue;
			}

			repository->findReferrerStorages(subject, collector_);
			break;
		}

	}

	return true;
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

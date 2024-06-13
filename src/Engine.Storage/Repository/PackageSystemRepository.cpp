#include "PackageSystemRepository.hpp"

#include <Engine.Reflect/CompileString.hpp>

#include "Repository.inl.hpp"
#include "../IStorage.hpp"
#include "../Options/StorageDescriptor.hpp"
#include "../Provider/PackageSystemProvider.hpp"
#include "../Storage/PackageStorage.hpp"
#include "../Url/Scheme.hpp"
#include "../Url/Url.hpp"

using namespace hg::engine::storage::system;
using namespace hg;

PackageSystemRepository::PackageSystemRepository(ref<PackageSystemProvider> provider_) noexcept :
	IStorageRepository(),
	_spp(std::addressof(provider_)) {}

PackageSystemRepository::~PackageSystemRepository() = default;

std::span<const nmpt<const IStorageProvider>> PackageSystemRepository::getProviders() const noexcept {
	return { std::addressof(static_cast<const nmpt<const IStorageProvider>&>(_spp)), 1uLL };
}

std::span<const engine::storage::UrlScheme> PackageSystemRepository::getUrlScheme() const noexcept {
	return std::span { &PackageScheme, 1uLL };
}

std::span<const u64> PackageSystemRepository::getRegisteredUrlScheme() const noexcept {
	// Warning: We just assume that we register the "package" scheme with the fnv1a compile-time hash...
	constexpr static auto scheme = "package"_cs.hash();
	return std::span { &scheme, 1uLL };
}

Arci<engine::storage::IStorage> PackageSystemRepository::createStorage(mref<StorageDescriptor> descriptor_) {
	::hg::assertrt(descriptor_.is<PackageStorageDescriptor>());
	auto request = std::move(descriptor_).as<PackageStorageDescriptor>();

	const auto guid = request.packageGuid();
	const auto [it, success] = _storages.emplace(guid, _spp->makeStorageObject(std::move(request)));
	return it->second.into<IStorage>();
}

bool PackageSystemRepository::hasStorage(cref<Url> url_) const {
	return inlineHasStorage<PackageUrl>(_storages, url_);
}

Arci<engine::storage::IStorage> PackageSystemRepository::getStorageByUrl(cref<Url> url_) const {
	return inlineGetStorageByUrl<PackageUrl>(_storages, url_);
}

bool PackageSystemRepository::removeStorage(mref<nmpt<IStorage>> storage_) {
	return inlineRemoveStorage(_storages, std::move(storage_));
}

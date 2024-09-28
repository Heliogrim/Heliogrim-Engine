#include "ArchiveSystemRepository.hpp"

#include <Engine.Reflect/CompileString.hpp>

#include "Repository.inl.hpp"
#include "../IStorage.hpp"
#include "../Options/StorageDescriptor.hpp"
#include "../Provider/PackageSystemProvider.hpp"
#include "../Storage/ArchiveStorage.hpp"
#include "../Url/Scheme.hpp"
#include "../Url/Url.hpp"

using namespace hg::engine::storage::system;
using namespace hg;

ArchiveSystemRepository::ArchiveSystemRepository(ref<PackageSystemProvider> provider_) noexcept :
	IStorageRepository(),
	_spp(std::addressof(provider_)) {}

ArchiveSystemRepository::~ArchiveSystemRepository() = default;

std::span<const nmpt<const IStorageProvider>> ArchiveSystemRepository::getProviders() const noexcept {
	return { std::bit_cast<ptr<const nmpt<const IStorageProvider>>>(std::addressof(_spp)), 1uLL };
}

std::span<const engine::storage::UrlScheme> ArchiveSystemRepository::getUrlScheme() const noexcept {
	return std::span { &ArchiveScheme, 1uLL };
}

std::span<const u64> ArchiveSystemRepository::getRegisteredUrlScheme() const noexcept {
	// Warning: We just assume that we register the "archive" scheme with the fnv1a compile-time hash...
	constexpr static auto scheme = "archive"_cs.hash();
	return std::span { &scheme, 1uLL };
}

Arci<engine::storage::IStorage> ArchiveSystemRepository::createStorage(mref<StorageDescriptor> descriptor_) {
	::hg::assertrt(descriptor_.is<ArchiveStorageDescriptor>());
	auto request = std::move(descriptor_).as<ArchiveStorageDescriptor>();

	const auto guid = request.archiveGuid();
	const auto [it, success] = _storages.emplace(guid, _spp->makeStorageObject(std::move(request)));
	return it->second.into<IStorage>();
}

bool ArchiveSystemRepository::hasStorage(cref<Url> url_) const {
	return inlineHasStorage<ArchiveUrl>(_storages, url_);
}

Arci<engine::storage::IStorage> ArchiveSystemRepository::getStorageByUrl(cref<Url> url_) const {
	return inlineGetStorageByUrl<ArchiveUrl>(_storages, url_);
}

bool ArchiveSystemRepository::removeStorage(mref<nmpt<IStorage>> storage_) {
	return inlineRemoveStorage(_storages, std::move(storage_));
}

#include "ArchiveSystemRepository.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Reflect/CompileString.hpp>

#include "../Provider/PackageSystemProvider.hpp"
#include "../Storage/ArchiveStorage.hpp"
#include "../Url/Scheme.hpp"

using namespace hg::engine::storage::system;
using namespace hg;

ArchiveSystemRepository::ArchiveSystemRepository(ref<PackageSystemProvider> provider_) noexcept :
	IStorageRepository(),
	_spp(std::addressof(provider_)) {}

ArchiveSystemRepository::~ArchiveSystemRepository() = default;

std::span<const nmpt<const IStorageProvider>> ArchiveSystemRepository::getProviders() const noexcept {
	return { std::addressof(static_cast<const nmpt<const IStorageProvider>&>(_spp)), 1uLL };
}

std::span<const engine::storage::UrlScheme> ArchiveSystemRepository::getUrlScheme() const noexcept {
	return std::span { &ArchiveScheme, 1uLL };
}

std::span<const u64> ArchiveSystemRepository::getRegisteredUrlScheme() const noexcept {
	// Warning: We just assume that we register the "mem" scheme with the fnv1a compile-time hash...
	constexpr static auto scheme = "archive"_cs.hash();
	return std::span { &scheme, 1uLL };
}

Arci<engine::storage::IStorage> ArchiveSystemRepository::createStorage(mref<StorageDescriptor> descriptor_) {
	::hg::todo_panic();
}

bool ArchiveSystemRepository::hasStorage(cref<Url> url_) const {
	::hg::todo_panic();
}

Arci<engine::storage::IStorage> ArchiveSystemRepository::getStorageByUrl(cref<Url> url_) const {
	::hg::todo_panic();
}

bool ArchiveSystemRepository::removeStorage(mref<nmpt<IStorage>> storage_) {
	::hg::todo_panic();
}

#include "PackageSystemRepository.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Reflect/CompileString.hpp>

#include "../Provider/PackageSystemProvider.hpp"
#include "../Url/Scheme.hpp"

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
	// Warning: We just assume that we register the "mem" scheme with the fnv1a compile-time hash...
	constexpr static auto scheme = "package"_cs.hash();
	return std::span { &scheme, 1uLL };
}

Arci<engine::storage::IStorage> PackageSystemRepository::createStorage(mref<StorageDescriptor> descriptor_) {
	::hg::todo_panic();
}

bool PackageSystemRepository::hasStorage(cref<Url> url_) const {
	return false;
}

Arci<engine::storage::IStorage> PackageSystemRepository::getStorageByUrl(cref<Url> url_) const {
	::hg::todo_panic();
}

bool PackageSystemRepository::removeStorage(mref<nmpt<IStorage>> storage_) {
	return false;
}

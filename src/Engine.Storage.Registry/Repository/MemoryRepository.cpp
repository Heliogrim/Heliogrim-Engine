#include "MemoryRepository.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Reflect/CompileString.hpp>

#include "../IStorage.hpp"
#include "../Options/StorageDescriptor.hpp"
#include "../Provider/MemoryProvider.hpp"
#include "../Storage/MemoryStorage.hpp"
#include "../Url/Scheme.hpp"
#include "../Url/Url.hpp"

using namespace hg::engine::storage::system;
using namespace hg;

MemoryRepository::MemoryRepository(ref<MemoryProvider> provider_) :
	IStorageRepository(),
	_provider(std::addressof(provider_)),
	_storages() {}

MemoryRepository::~MemoryRepository() = default;

std::span<const nmpt<const IStorageProvider>> MemoryRepository::getProviders() const noexcept {
	return { std::bit_cast<ptr<const nmpt<const IStorageProvider>>>(std::addressof(_provider)), 1uLL };
}

std::span<const engine::storage::UrlScheme> MemoryRepository::getUrlScheme() const noexcept {
	return std::span { &MemoryScheme, 1uLL };
}

std::span<const u64> MemoryRepository::getRegisteredUrlScheme() const noexcept {
	// Warning: We just assume that we register the "mem" scheme with the fnv1a compile-time hash...
	constexpr static auto scheme = "mem"_cs.hash();
	return std::span { &scheme, 1uLL };
}

Arci<engine::storage::IStorage> MemoryRepository::createStorage(mref<StorageDescriptor> descriptor_) {
	::hg::assertrt(descriptor_.is<MemoryStorageDescriptor>());
	auto request = std::move(descriptor_).as<MemoryStorageDescriptor>();

	const auto guid = request.memoryGuid();
	const auto [it, success] = _storages.emplace(guid, _provider->makeStorageObject(std::move(request)));
	return it->second.into<IStorage>();
}

bool MemoryRepository::hasStorage(cref<Url> url_) const {
	return url_.is<MemoryUrl>() && _storages.contains(url_.as<MemoryUrl>().guid());
}

Arci<engine::storage::IStorage> MemoryRepository::getStorageByUrl(cref<Url> url_) const {
	if (not url_.is<MemoryUrl>())
		return {};

	const auto it = _storages.find(url_.as<MemoryUrl>().guid());
	return it != _storages.end() ? it->second.into<IStorage>() : Arci<engine::storage::IStorage> {};
}

void MemoryRepository::findReferrerStorages(cref<Arci<IStorage>> ref_, ref<Vector<Arci<IStorage>>> collector_) const {
	// Note: Memory storage is never backed by any other storage.
}

bool MemoryRepository::removeStorage(mref<nmpt<IStorage>> storage_) {
	const auto vecIt = std::ranges::find(
		_storages.values(),
		storage_.get(),
		[](const auto& pair_) {
			return pair_.second.get();
		}
	);
	return vecIt != _storages.values().end() && _storages.erase(vecIt->first);
}

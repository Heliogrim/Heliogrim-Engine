#include "RuntimeFileSystemRepository.hpp"

#include <array>
#include <Engine.Filesystem/Url.hpp>
#include <Engine.Reflect/Compile/String.hpp>

#include "../IStorage.hpp"
#include "../Options/StorageDescriptor.hpp"
#include "../Provider/LocalFileSystemProvider.hpp"
#include "../Storage/LocalFileStorage.hpp"
#include "../Url/StorageUrl.hpp"

using namespace hg::engine::storage::system;
using namespace hg;

RuntimeFileSystemRepository::RuntimeFileSystemRepository(
	ref<LocalFileSystemProvider> provider_,
	mref<hg::fs::File::path_type> basePath_
) noexcept :
	IStorageFileRepository(),
	_lfs(std::addressof(provider_)),
	_basePath(std::move(basePath_)) {}

RuntimeFileSystemRepository::~RuntimeFileSystemRepository() = default;

std::span<const nmpt<const IStorageProvider>> RuntimeFileSystemRepository::getProviders() const noexcept {
	return { std::bit_cast<ptr<const nmpt<const IStorageProvider>>>(std::addressof(_lfs)), 1uLL };
}

std::span<const engine::storage::UrlScheme> RuntimeFileSystemRepository::getUrlScheme() const noexcept {
	constexpr static auto scheme = std::array<UrlScheme, 2uLL> { FileRuntimeScheme, FileScheme };
	return std::span { scheme.data(), 2uLL };
}

std::span<const u64> RuntimeFileSystemRepository::getRegisteredUrlScheme() const noexcept {
	// Warning: We just assume that we register the "mem" scheme with the fnv1a compile-time hash...
	constexpr static auto scheme = std::array<u64, 2uLL> { "file+runtime"_cs.hash(), "file"_cs.hash() };
	return std::span { scheme.data(), 2uLL };
}

StringView RuntimeFileSystemRepository::getVfsMountPoint() const noexcept {
	return {};
}

Arci<engine::storage::IStorage> RuntimeFileSystemRepository::createStorage(mref<StorageDescriptor> descriptor_) {
	::hg::assertrt(std::holds_alternative<FileStorageDescriptor>(descriptor_));

	auto lfsStore = descriptor_.as<FileStorageDescriptor>().url.path();
	const auto [it, success] = _storages.emplace(
		std::get<FileStorageDescriptor>(std::move(descriptor_)).url.path(),
		_lfs->makeStorageObject(std::move(lfsStore))
	);
	return it->second.into<IStorage>();
}

bool RuntimeFileSystemRepository::hasStorage(cref<StorageUrl> url_) const {
	if (not url_.is<FileUrl>())
		return false;

	const auto it = _storages.find(static_cast<String>(std::get<FileUrl>(url_).path()));
	return it != _storages.end();
}

Arci<engine::storage::IStorage> RuntimeFileSystemRepository::getStorageByUrl(cref<StorageUrl> url_) const {
	if (not url_.is<FileUrl>())
		return {};

	const auto it = _storages.find(static_cast<String>(std::get<FileUrl>(url_).path()));
	return it != _storages.end() ? it->second.into<IStorage>() : Arci<engine::storage::IStorage> {};
}

void RuntimeFileSystemRepository::findReferrerStorages(cref<Arci<IStorage>> ref_, ref<Vector<Arci<IStorage>>> collector_) const {
	// Note: filesystem storage is never backed by any other storage.
}

bool RuntimeFileSystemRepository::removeStorage(mref<nmpt<IStorage>> storage_) {
	const auto vecIt = std::ranges::find(
		_storages.values(),
		storage_.get(),
		[](const auto& pair_) {
			return pair_.second.get();
		}
	);
	if (vecIt != _storages.values().end()) {
		return _storages.erase(vecIt->first);
	}
	return false;
}

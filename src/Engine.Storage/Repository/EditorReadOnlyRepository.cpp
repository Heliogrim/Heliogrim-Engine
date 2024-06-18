#include "EditorReadOnlyRepository.hpp"

#include <array>
#include <Engine.Filesystem/Url.hpp>
#include <Engine.Reflect/CompileString.hpp>

#include "../IStorage.hpp"
#include "../Options/StorageDescriptor.hpp"
#include "../Provider/LocalFileSystemProvider.hpp"
#include "../Storage/LocalFileStorage.hpp"
#include "../Url/Url.hpp"

using namespace hg::engine::storage::system;
using namespace hg;

EditorReadOnlyRepository::EditorReadOnlyRepository(
	ref<LocalFileSystemProvider> provider_,
	mref<hg::fs::File::path_type> basePath_
) :
	IStorageFileRepository(),
	_lfs(std::addressof(provider_)),
	_basePath(std::move(basePath_)) {}

EditorReadOnlyRepository::~EditorReadOnlyRepository() = default;

std::span<const nmpt<const IStorageProvider>> EditorReadOnlyRepository::getProviders() const noexcept {
	return { std::bit_cast<ptr<const nmpt<const IStorageProvider>>>(std::addressof(_lfs)), 1uLL };
}

std::span<const engine::storage::UrlScheme> EditorReadOnlyRepository::getUrlScheme() const noexcept {
	constexpr auto scheme = std::array<UrlScheme, 2uLL> { FileEditorScheme, FileScheme };
	return std::span { scheme.data(), 2uLL };
}

std::span<const u64> EditorReadOnlyRepository::getRegisteredUrlScheme() const noexcept {
	// Warning: We just assume that we register the "mem" scheme with the fnv1a compile-time hash...
	constexpr auto scheme = std::array<u64, 2uLL> { "file+editor"_cs.hash(), "file"_cs.hash() };
	return std::span { scheme.data(), 2uLL };
}

StringView EditorReadOnlyRepository::getVfsMountPoint() const noexcept {
	return {};
}

Arci<engine::storage::IStorage> EditorReadOnlyRepository::createStorage(mref<StorageDescriptor> descriptor_) {
	::hg::assertrt(std::holds_alternative<FileStorageDescriptor>(descriptor_));
	const auto [it, success] = _storages.emplace(
		std::get<FileStorageDescriptor>(std::move(descriptor_)).url.path().string(),
		_lfs->makeStorageObject()
	);
	return it->second.into<IStorage>();
}

bool EditorReadOnlyRepository::hasStorage(cref<Url> url_) const {
	if (not url_.is<FileUrl>())
		return false;

	const auto it = _storages.find(url_.as<FileUrl>().path().string());
	return it != _storages.end();
}

Arci<engine::storage::IStorage> EditorReadOnlyRepository::getStorageByUrl(cref<Url> url_) const {
	if (not url_.is<FileUrl>())
		return {};

	const auto it = _storages.find(url_.as<FileUrl>().path().string());
	return it != _storages.end() ? it->second.into<IStorage>() : Arci<engine::storage::IStorage> {};
}

bool EditorReadOnlyRepository::removeStorage(mref<nmpt<IStorage>> storage_) {
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

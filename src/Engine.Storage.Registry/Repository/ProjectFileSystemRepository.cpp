#include "ProjectFileSystemRepository.hpp"

#include <array>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Env/Check.hpp>
#include <Engine.Filesystem/Url.hpp>
#include <Engine.Reflect/CompileString.hpp>

#include "../IStorage.hpp"
#include "../Options/StorageDescriptor.hpp"
#include "../Provider/LocalFileSystemProvider.hpp"
#include "../Storage/LocalFileStorage.hpp"
#include "../Url/Url.hpp"

using namespace hg::engine::storage::system;
using namespace hg;

/**/

[[nodiscard]] static bool is_sub_path(cref<std::filesystem::path> base_, cref<std::filesystem::path> check_);

/**/

ProjectFileSystemRepository::ProjectFileSystemRepository(
	ref<LocalFileSystemProvider> provider_,
	mref<hg::fs::File::path_type> basePath_
) noexcept :
	IStorageFileRepository(),
	_lfs(std::addressof(provider_)),
	_basePath(std::move(basePath_)) {}

ProjectFileSystemRepository::~ProjectFileSystemRepository() = default;

std::span<const nmpt<const IStorageProvider>> ProjectFileSystemRepository::getProviders() const noexcept {
	return { std::bit_cast<ptr<const nmpt<const IStorageProvider>>>(std::addressof(_lfs)), 1uLL };
}

std::span<const engine::storage::UrlScheme> ProjectFileSystemRepository::getUrlScheme() const noexcept {
	constexpr static auto scheme = std::array<UrlScheme, 2uLL> { FileProjectScheme, FileScheme };
	return std::span { scheme.data(), 2uLL };
}

std::span<const u64> ProjectFileSystemRepository::getRegisteredUrlScheme() const noexcept {
	// Warning: We just assume that we register the "mem" scheme with the fnv1a compile-time hash...
	constexpr static auto scheme = std::array<u64, 2uLL> { "file+project"_cs.hash(), "file"_cs.hash() };
	return std::span { scheme.data(), 2uLL };
}

StringView ProjectFileSystemRepository::getVfsMountPoint() const noexcept {
	return {};
}

Arci<engine::storage::IStorage> ProjectFileSystemRepository::createStorage(mref<StorageDescriptor> descriptor_) {
	::hg::assertrt(std::holds_alternative<FileStorageDescriptor>(descriptor_));

	auto lfsStore = descriptor_.as<FileStorageDescriptor>().url.path();
	const auto [it, success] = _storages.emplace(
		std::get<FileStorageDescriptor>(std::move(descriptor_)).url.path(),
		_lfs->makeStorageObject(std::move(lfsStore))
	);
	return it->second.into<IStorage>();
}

bool ProjectFileSystemRepository::hasStorage(cref<Url> url_) const {
	if (not url_.is<FileUrl>())
		return false;

	const auto it = _storages.find(static_cast<String>(url_.as<FileUrl>().path()));

	/**/

	if constexpr (env::check<env::EnvProps::eIsEditor>()) {
		if (it != _storages.end()) {
			return true;
		}

		const auto lookup = [](const auto& url_, const auto& basePath_) {
			const auto& stdFsPath = static_cast<cref<std::filesystem::path>>(url_.template as<FileUrl>().path());
			if (stdFsPath.is_absolute() && is_sub_path(basePath_, stdFsPath)) {
				return stdFsPath;
			}
			return clone(static_cast<cref<std::filesystem::path>>(basePath_)).append(stdFsPath.string());
		}(url_, _basePath);

		return std::filesystem::exists(lookup);
	}

	/**/

	return it != _storages.end();
}

Arci<engine::storage::IStorage> ProjectFileSystemRepository::getStorageByUrl(cref<Url> url_) const {
	if (not url_.is<FileUrl>())
		return {};

	const auto it = _storages.find(static_cast<String>(url_.as<FileUrl>().path()));

	/**/

	// TODO: Check whether we can mark the difference of editor and runtime in a better way.
	if constexpr (env::check<env::EnvProps::eIsEditor>()) {
		if (it != _storages.end()) {
			return it->second.into<IStorage>();
		}

		auto lookup = [](const auto& url_, const auto& basePath_) {
			const auto& stdFsPath = static_cast<cref<std::filesystem::path>>(url_.template as<FileUrl>().path());
			if (stdFsPath.is_absolute() && is_sub_path(basePath_, stdFsPath)) {
				return stdFsPath;
			}
			return clone(static_cast<cref<std::filesystem::path>>(basePath_)).append(stdFsPath.string());
		}(url_, _basePath);

		if (std::filesystem::exists(lookup)) {

			// Problem: The following upsert will break the constness rules and may cause corruption in a concurrent environment
			return const_cast<ref<ProjectFileSystemRepository>>(*this).createStorage(
				FileStorageDescriptor { FileUrl { clone(FileScheme), fs::Path { std::move(lookup) } } }
			);
		}

		return Arci<engine::storage::IStorage> {};
	}

	/**/

	return it != _storages.end() ? it->second.into<IStorage>() : Arci<engine::storage::IStorage> {};
}

void ProjectFileSystemRepository::findReferrerStorages(cref<Arci<IStorage>> ref_, ref<Vector<Arci<IStorage>>> collector_) const {
	// Note: filesystem storage is never backed by any other storage.
}

bool ProjectFileSystemRepository::removeStorage(mref<nmpt<IStorage>> storage_) {
	const auto vecIt = std::ranges::find(
		_storages.values(),
		storage_.get(),
		[](const auto& pair_) { return pair_.second.get(); }
	);
	if (vecIt != _storages.values().end()) {
		return _storages.erase(vecIt->first);
	}
	return false;
}

/**/

bool is_sub_path(cref<std::filesystem::path> base_, cref<std::filesystem::path> check_) {
	const auto rel = std::filesystem::relative(check_, base_);
	return rel.empty() || rel.native().front() != '.';
}

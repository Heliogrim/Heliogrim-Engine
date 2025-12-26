#include "FilesystemBrowserProvider.hpp"

#include <filesystem>
#include <Engine.Common/Move.hpp>

#include "AssetBrowserFilter.hpp"

using namespace hg::editor::ui::service;
using namespace hg;

FilesystemBrowserProvider::FilesystemBrowserProvider() :
	AssetBrowserProvider(),
	_root(
		"file"sv,
		fs::Path {
			/* <root> */
		}
	) {}

string FilesystemBrowserProvider::normalizeFsPath(cref<std::filesystem::path> unnormalized_) const {

	const std::filesystem::path root = _root.path();

	auto cpy = unnormalized_;
	if (cpy.has_stem()) {

		if (cpy.stem() == root.stem()) {
			return "";
		}

		cpy = std::filesystem::relative(::hg::move(cpy), root);
	}

	if (cpy.wstring().starts_with(L"..")) {
		return "";
	}

	return cpy.string();
}

std::filesystem::path FilesystemBrowserProvider::expandPath(mref<std::filesystem::path> normalized_) const {

	std::filesystem::path tmp = _root.path();
	if (normalized_.has_root_path()) {
		tmp += ::hg::move(normalized_);
	} else {
		tmp /= ::hg::move(normalized_);
	}

	return tmp;
}

bool FilesystemBrowserProvider::retrieveFs(
	cref<fs::Url> url_,
	const bool directories_,
	ref<Vector<AssetBrowserEntry>> entries_
) const {
	const auto fsPath = expandPath(string { url_.path() });
	if (not std::filesystem::is_directory(fsPath)) {
		return false;
	}

	const auto fsIt = std::filesystem::directory_iterator {
		fsPath,
		std::filesystem::directory_options::follow_directory_symlink
	};

	for (const auto& fsEntry : fsIt) {

		if (directories_ && not fsEntry.is_directory()) {
			continue;
		}

		if (not directories_ && fsEntry.is_directory()) {
			continue;
		}

		auto subPath = normalizeFsPath(fsEntry.path());
		entries_.push_back(
			AssetBrowserEntry {
				.type = (fsEntry.is_directory() ? AssetBrowserEntryType::eDirectory : AssetBrowserEntryType::eFile),
				.url = { engine::assets::AssetPath { fs::Path { fsEntry.path() } }, engine::assets::AssetName {} },
				.guid = {}
			}
		);
	}

	return true;

}

bool FilesystemBrowserProvider::effects(ref<const AssetBrowserFilter> filter_) const {

	if (not filter_.url.hasScheme()) {
		return true;
	}

	return filter_.url.scheme() == "file"sv;
}

bool FilesystemBrowserProvider::fetchItems(ref<const AssetBrowserFilter> filter_, ref<Vector<AssetBrowserEntry>> entries_) const {
	return retrieveFs(filter_.url, false, entries_);
}

bool FilesystemBrowserProvider::fetchDirectories(ref<const AssetBrowserFilter> filter_, ref<Vector<AssetBrowserEntry>> directories_) const {
	return retrieveFs(filter_.url, true, directories_);
}

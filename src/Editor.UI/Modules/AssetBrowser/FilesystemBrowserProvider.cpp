#include "FilesystemBrowserProvider.hpp"

#include <filesystem>

using namespace hg::editor::ui;
using namespace hg;

FilesystemBrowserProvider::FilesystemBrowserProvider() :
	AssetBrowserProvider(),
	_root(
		"file"sv,
		fs::Path {
			/* <root> */
		}
	) {}

string FilesystemBrowserProvider::normalizeFsPath(mref<string> unnormalized_) {

	const std::filesystem::path root = _root.path();
	std::filesystem::path tmp { std::move(unnormalized_) };

	if (tmp.has_stem()) {

		if (tmp.stem() == root.stem()) {
			return "";
		}

		tmp = std::filesystem::relative(tmp, root);
	}

	if (tmp.wstring().starts_with(L"..")) {
		return "";
	}

	return tmp.string();
}

string FilesystemBrowserProvider::expandPath(mref<string> normalized_) {

	std::filesystem::path tmp = _root.path();
	const std::filesystem::path norm = std::move(normalized_);

	if (norm.has_root_path()) {
		tmp += norm;
	} else {
		tmp /= norm;
	}

	return tmp.string();
}

bool FilesystemBrowserProvider::retrieveFs(
	cref<fs::Url> url_,
	const bool directories_,
	ref<Vector<AssetBrowserEntry>> entries_
) {
	const std::filesystem::path fsPath = expandPath(string { url_.path() });
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

		auto subPath = normalizeFsPath(fsEntry.path().string());
		entries_.push_back(
			AssetBrowserEntry {
				.type = (fsEntry.is_directory() ? AssetBrowserEntryType::eDirectory : AssetBrowserEntryType::eFile),
				.title = fsEntry.path().filename().string(),
				.path = fs::Url { "file"sv, std::move(subPath) }
			}
		);
	}

	return true;

}

bool FilesystemBrowserProvider::effects(cref<fs::Url> url_) {

	if (not url_.hasScheme()) {
		return true;
	}

	return url_.scheme() == "file"sv;
}

bool FilesystemBrowserProvider::retrieve(cref<fs::Url> url_, ref<Vector<AssetBrowserEntry>> entries_) {
	return retrieveFs(url_, false, entries_);
}

bool FilesystemBrowserProvider::retrieveDirectories(cref<fs::Url> url_, ref<Vector<AssetBrowserEntry>> directories_) {
	return retrieveFs(url_, true, directories_);
}

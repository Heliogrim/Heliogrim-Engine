#include "FilesystemBrowserProvider.hpp"

#include <filesystem>

#include "Engine.Resource/FileResource.hpp"

using namespace hg::editor::ui;
using namespace hg;

FilesystemBrowserProvider::FilesystemBrowserProvider() :
    AssetBrowserProvider(),
    _root("file"sv, FileResource::getRootDir()) {}

string FilesystemBrowserProvider::normalizeFsPath(mref<string> unnormalized_) {

    const _STD filesystem::path root { _root.path() };
    _STD filesystem::path tmp { _STD move(unnormalized_) };

    if (tmp.has_stem()) {

        if (tmp.stem() == root.stem()) {
            return "";
        }

        tmp = _STD filesystem::relative(tmp, root);
    }

    if (tmp.wstring().starts_with(L"..")) {
        return "";
    }

    return tmp.string();
}

string FilesystemBrowserProvider::expandPath(mref<string> normalized_) {

    _STD filesystem::path tmp = _root.path();
    const _STD filesystem::path norm = _STD move(normalized_);

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
    const _STD filesystem::path fsPath = expandPath(string { url_.path() });
    if (not _STD filesystem::is_directory(fsPath)) {
        return false;
    }

    const auto fsIt = _STD filesystem::directory_iterator {
        fsPath,
        _STD filesystem::directory_options::follow_directory_symlink
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
                .path = fs::Url { "file"sv, _STD move(subPath) }
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

#include "AssetBrowser.hpp"

#include <Engine.Common/stdafx.h>
#include "../Panel/AssetBrowserPanel.hpp"

using namespace ember::editor::ui;
using namespace ember;

AssetBrowser::AssetBrowser() {

    /**/
    const auto root { getBrowserRoot() };
    const auto fsRoot { getVirtProtoRoot("file"sv, 0ui64) };

    const string rootPath { root.path() };
    _virtualMountPoints.insert_or_assign(rootPath, Vector<_STD pair<string, Url>> {
        _STD pair<string, Url> {
            "Audio",
            Url {
                "file"sv,
                _STD filesystem::path { fsRoot.path() }.append(R"(..\..\resources\assets\audio)").string()
            }
        },
        _STD pair<string, Url> {
            "Font",
            Url {
                "file"sv,
                _STD filesystem::path { fsRoot.path() }.append(R"(..\..\resources\assets\font)").string()
            }
        },
        _STD pair<string, Url> {
            "Geometry",
            Url {
                "file"sv,
                _STD filesystem::path { fsRoot.path() }.append(R"(..\..\resources\assets\geometry)").string()
            }
        },
        _STD pair<string, Url> {
            "Material",
            Url {
                "file"sv,
                _STD filesystem::path { fsRoot.path() }.append(R"(..\..\resources\assets\material)").string()
            }
        },
        _STD pair<string, Url> {
            "Scene",
            Url {
                "file"sv,
                _STD filesystem::path { fsRoot.path() }.append(R"(..\..\resources\world)").string()
            }
        },
        _STD pair<string, Url> {
            "Shader",
            Url {
                "file"sv,
                _STD filesystem::path { fsRoot.path() }.append(R"(..\..\resources\shader)").string()
            }
        },
        _STD pair<string, Url> {
            "Texture",
            Url {
                "file"sv,
                _STD filesystem::path { fsRoot.path() }.append(R"(..\..\resources\assets\texture)").string()
            }
        },
        _STD pair<string, Url> {
            "Imports",
            Url {
                "file"sv,
                _STD filesystem::path { fsRoot.path() }.append(R"(..\..\resources\imports)").string()
            }
        },
    });
    /**/
}

AssetBrowser::~AssetBrowser() {}

sptr<AssetBrowserPanel> AssetBrowser::makePanel() {

    const auto root { getBrowserRoot() };

    /**/

    auto panel { AssetBrowserPanel::make(this, root) };
    _panels.push_back(panel);
    return panel;
}

Url AssetBrowser::getBrowserRoot() const noexcept {
    return Url { ""sv, "/"sv };
}

Url AssetBrowser::getVirtProtoRoot(cref<string_view> schema_, const u64 virtRootId_) {
    assert(schema_ == "file"sv);
    assert(virtRootId_ == 0);

    const _STD filesystem::path cwd { _STD filesystem::current_path() };
    const Url cwdUrl { "file"sv, cwd.string() };
    return cwdUrl;
}

Url AssetBrowser::getImportRoot() const noexcept {
    return Url { ""sv, "//Imports"sv };
}

bool AssetBrowser::retrieveEntriesFQUrl(cref<Url> fqurl_, ref<Vector<std::pair<string, Url>>> entries_) const {

    if (!fqurl_.hasScheme() || fqurl_.scheme() != "file"sv) {
        // TODO: Implement schema dependent data lookup
        return false;
    }

    if (fqurl_.hasScheme() && fqurl_.scheme() == "file"sv) {
        // TODO: Resolve local file system data

        const auto fsResolved { _STD filesystem::path { fqurl_.path() } };
        if (not _STD filesystem::is_directory(fsResolved)) {
            return false;
        }

        const auto fsIter {
            _STD filesystem::directory_iterator {
                fsResolved,
                _STD filesystem::directory_options::follow_directory_symlink
            }
        };
        for (const auto& fsEntry : fsIter) {
            entries_.push_back(_STD pair<string, Url> {
                fsEntry.path().filename().string(),
                Url { "file"sv, fsEntry.path().string() }
            });
        }

    }

    return true;
}

bool AssetBrowser::retrieveEntries(cref<Url> cwd_, ref<Vector<_STD pair<string, Url>>> entries_) const {

    if (cwd_.hasScheme()) {
        return retrieveEntriesFQUrl(cwd_, entries_);
    }

    /**/

    const auto root { getBrowserRoot() };

    // Warning: Just a small sanitize to prevent root mismatch
    const bool isRoot {
        cwd_.path() == root.path() ||
        (cwd_.path().size() == 2 && cwd_.path().ends_with(Url::Separator))
    };

    if (not cwd_.hasScheme() && isRoot) {

        const auto virtMappedIter { _virtualMountPoints.find(string { root.path() }) };
        if (virtMappedIter == _virtualMountPoints.end()) {
            DEBUG_MSG("Failed to find any virtual mounting points for virtual browser root ?!?")
            return true;
        }

        const auto virtMapped { virtMappedIter->second };
        entries_.insert(entries_.end(), virtMapped.begin(), virtMapped.end());

        return true;
    }

    /**/

    auto proxyCwd { _STD filesystem::path { cwd_.path() } };
    Vector<string> proxyCwdParts {};
    while (not proxyCwd.empty() && proxyCwd != proxyCwd.parent_path()) {
        proxyCwdParts.push_back(proxyCwd.filename().string());
        proxyCwd = proxyCwd.parent_path();
    }

    if (proxyCwd.has_root_name() && proxyCwd.root_name().native().size() != root.path().size()) {
        proxyCwdParts.push_back(proxyCwd.root_name().string().substr(root.path().size() + Url::Separator.size()));
    }

    proxyCwdParts.push_back(string { root.path() });

    /**/

    Vector<Url> virtResolve {};

    string nextPart { proxyCwdParts.back() };
    Url fwdUrl { ""sv, nextPart };

    proxyCwdParts.pop_back();
    virtResolve.push_back(fwdUrl);

    do {

        nextPart = proxyCwdParts.back();

        /**/

        Vector<_STD pair<string, Url>> partEntries {};
        for (const auto& partial : virtResolve) {
            const auto result = retrieveEntries(partial, partEntries);
            assert(result);
        }

        /**/

        virtResolve.clear();
        for (const auto& entry : partEntries) {
            if (entry.first == nextPart) {
                virtResolve.push_back(entry.second);
            }
        }

        /**/

        fwdUrl = Url { ""sv, string { fwdUrl.path() }.append(Url::Separator).append(nextPart) };
        proxyCwdParts.pop_back();

    } while (not proxyCwdParts.empty());

    /**/

    for (const auto& resolved : virtResolve) {

        if (resolved.hasScheme()) {

            const auto result = retrieveEntriesFQUrl(resolved, entries_);
            assert(result);

        } else {

            const auto virtMappedIt { _virtualMountPoints.find(string { resolved.path() }) };
            if (virtMappedIt == _virtualMountPoints.end()) {
                continue;
            }

            entries_.insert(entries_.end(), virtMappedIt->second.begin(), virtMappedIt->second.end());
        }

    }

    return true;
}

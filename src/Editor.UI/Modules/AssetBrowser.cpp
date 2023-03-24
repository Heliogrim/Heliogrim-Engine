#include "AssetBrowser.hpp"

#include <Engine.Assets/Assets.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Logging/Logger.hpp>

#include "../Panel/AssetBrowserPanel.hpp"
#include "AssetBrowser/AssetRegistryBrowserProvider.hpp"
#include "AssetBrowser/FilesystemBrowserProvider.hpp"

using namespace hg::editor::ui;
using namespace hg;

AssetBrowser::AssetBrowser() {

    const auto* const engine = engine::Engine::getEngine();

    _providers.push_back(make_uptr<AssetRegistryBrowserProvider>(engine->getAssets()->getRegistry()));
    _providers.push_back(make_uptr<FilesystemBrowserProvider>());
}

AssetBrowser::~AssetBrowser() = default;

sptr<AssetBrowserPanel> AssetBrowser::makePanel() {

    const auto root { getBrowserRoot() };

    /**/

    auto panel { AssetBrowserPanel::make(this, root) };
    _panels.push_back(panel);
    return panel;
}

void AssetBrowser::addProvider(mref<uptr<AssetBrowserProvider>> provider_) {
    _providers.push_back(_STD move(provider_));
}

fs::Url AssetBrowser::getBrowserRoot() const noexcept {
    return fs::Url {};
}

fs::Url AssetBrowser::getImportRoot() const noexcept {
    return fs::Url { fs::Path { "/Imports"sv } };
}

void AssetBrowser::retrieveFromProviders(cref<fs::Url> url_, ref<Vector<AssetBrowserEntry>> entries_) const {
    for (const auto& provider : _providers) {
        if (provider->effects(url_)) {
            provider->retrieve(url_, entries_);
        }
    }
}

void AssetBrowser::retrieveProviderDirectories(cref<fs::Url> url_, ref<Vector<AssetBrowserEntry>> directories_) const {
    for (const auto& provider : _providers) {
        if (provider->effects(url_)) {
            provider->retrieveDirectories(url_, directories_);
        }
    }
}

bool AssetBrowser::retrieveDirectories(cref<fs::Url> cwd_, ref<Vector<AssetBrowserEntry>> directories_) const {
    retrieveProviderDirectories(cwd_, directories_);
    return true;
}

bool AssetBrowser::retrieveEntries(cref<fs::Url> cwd_, ref<Vector<AssetBrowserEntry>> entries_) const {

    const auto root { getBrowserRoot() };

    // Warning: Just a small sanitize to prevent root mismatch
    const bool isRoot {
        cwd_.empty() || cwd_.path() == root.path()
    };

    if (isRoot) {
        retrieveFromProviders(root, entries_);
        return true;
    }

    /**/

    Vector<AssetBrowserEntry> virtResolve {};
    virtResolve.push_back(
        AssetBrowserEntry {
            .type = AssetBrowserEntryType::eDirectory,
            .title = "",
            .path = cwd_
        }
    );

    /**/

    for (const auto& resolved : virtResolve) {

        if (resolved.type != AssetBrowserEntryType::eDirectory) {
            IM_CORE_ERROR("Found non-directory while virtually resolving paths for assets from asset browser.");
            continue;
        }

        /**/

        retrieveFromProviders(resolved.path, entries_);
    }

    return true;
}

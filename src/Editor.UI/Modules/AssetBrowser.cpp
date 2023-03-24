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

bool AssetBrowser::retrieveItems(cref<fs::Url> cwd_, ref<Vector<AssetBrowserEntry>> entries_) const {
    retrieveFromProviders(cwd_, entries_);
    return true;
}

void AssetBrowser::retrieveFromProviders(cref<fs::Url> url_, ref<Vector<AssetBrowserEntry>> entries_) const {
    for (const auto& provider : _providers) {
        if (provider->effects(url_)) {
            provider->retrieve(url_, entries_);
        }
    }
}

bool AssetBrowser::retrieveDirectories(cref<fs::Url> cwd_, ref<Vector<AssetBrowserEntry>> directories_) const {
    retrieveProviderDirectories(cwd_, directories_);
    return true;
}

void AssetBrowser::retrieveProviderDirectories(cref<fs::Url> url_, ref<Vector<AssetBrowserEntry>> directories_) const {
    for (const auto& provider : _providers) {
        if (provider->effects(url_)) {
            provider->retrieveDirectories(url_, directories_);
        }
    }
}

void AssetBrowser::retrieve(cref<fs::Url> cwd_, ref<Vector<AssetBrowserEntry>> entries_) {
    retrieveDirectories(cwd_, entries_);
    retrieveItems(cwd_, entries_);
}

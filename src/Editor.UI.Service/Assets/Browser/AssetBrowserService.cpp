#include "AssetBrowserService.hpp"

#include <Engine.Assets/Assets.hpp>
#include <Engine.Common/Move.hpp>
#include <Engine.Core/Engine.hpp>

#include "AssetBrowserEntry.hpp"
#include "AssetBrowserProvider.hpp"
#include "AssetRegistryBrowserProvider.hpp"

using namespace hg::editor::ui::service;
using namespace hg;

AssetBrowserService::AssetBrowserService() {
	const auto engine = engine::Engine::getEngine();
	_providers.emplace_back(make_uptr<AssetRegistryBrowserProvider>(engine->getAssets()->getRegistry()));
}

AssetBrowserService::~AssetBrowserService() = default;

void AssetBrowserService::addProvider(mref<UniquePtr<AssetBrowserProvider>> provider_) {
	_providers.emplace_back(::hg::move(provider_));
}

void AssetBrowserService::fetchProviderItems(ref<const fs::Url> url_, ref<Vector<AssetBrowserEntry>> items_) const {
	for (const auto& provider : _providers) {
		if (provider->effects(url_)) {
			provider->fetchItems(url_, items_);
		}
	}
}

void AssetBrowserService::fetchProviderDirectories(ref<const fs::Url> url_, ref<Vector<AssetBrowserEntry>> directories_) const {
	for (const auto& provider : _providers) {
		if (provider->effects(url_)) {
			provider->fetchDirectories(url_, directories_);
		}
	}
}

void AssetBrowserService::fetchAll(ref<const fs::Url> url_, ref<Vector<AssetBrowserEntry>> entries_) const {
	fetchProviderDirectories(url_, entries_);
	fetchProviderItems(url_, entries_);
}

void AssetBrowserService::fetchItems(ref<const fs::Url> url_, ref<Vector<AssetBrowserEntry>> items_) const {
	fetchProviderItems(url_, items_);
}

void AssetBrowserService::fetchDirectories(ref<const fs::Url> url_, ref<Vector<AssetBrowserEntry>> directories_) const {
	fetchProviderDirectories(url_, directories_);
}

#include "AssetBrowserService.hpp"

#include <Engine.Assets/Assets.hpp>
#include <Engine.Common/Move.hpp>
#include <Engine.Core/Engine.hpp>

#include "AssetBrowserEntry.hpp"
#include "AssetBrowserFilter.hpp"
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

void AssetBrowserService::fetchProviderItems(ref<const AssetBrowserFilter> filter_, ref<Vector<AssetBrowserEntry>> items_) const {
	for (const auto& provider : _providers) {
		if (provider->effects(filter_)) {
			provider->fetchItems(filter_, items_);
		}
	}
}

void AssetBrowserService::fetchProviderDirectories(
	ref<const AssetBrowserFilter> filter_,
	ref<Vector<AssetBrowserEntry>> directories_
) const {
	for (const auto& provider : _providers) {
		if (provider->effects(filter_)) {
			provider->fetchDirectories(filter_, directories_);
		}
	}
}

void AssetBrowserService::fetchAll(ref<const AssetBrowserFilter> filter_, ref<Vector<AssetBrowserEntry>> entries_) const {
	fetchProviderDirectories(filter_, entries_);
	fetchProviderItems(filter_, entries_);
}

void AssetBrowserService::fetchItems(ref<const AssetBrowserFilter> filter_, ref<Vector<AssetBrowserEntry>> items_) const {
	fetchProviderItems(filter_, items_);
}

void AssetBrowserService::fetchDirectories(ref<const AssetBrowserFilter> filter_, ref<Vector<AssetBrowserEntry>> directories_) const {
	fetchProviderDirectories(filter_, directories_);
}
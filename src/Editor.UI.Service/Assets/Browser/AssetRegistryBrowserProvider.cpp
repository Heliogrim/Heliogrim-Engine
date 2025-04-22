#include "AssetRegistryBrowserProvider.hpp"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <ranges>
#include <Engine.Assets.System/AssetRegistry.hpp>
#include <Engine.Assets.Type/Asset.hpp>

using namespace hg::editor::ui::service;
using namespace hg;

AssetRegistryBrowserProvider::AssetRegistryBrowserProvider(
	nmpt<engine::assets::IAssetRegistry> registry_
) noexcept :
	_registry(registry_) {}

AssetRegistryBrowserProvider::~AssetRegistryBrowserProvider() = default;

AssetRegistryBrowserProvider::data_list_type::iterator AssetRegistryBrowserProvider::filterByType(
	ref<const data_list_type::iterator> begin_,
	ref<const data_list_type::iterator> end_,
	const AssetTypeId typeId_
) const {

	const auto subrange = std::ranges::remove_if(
		begin_,
		end_,
		[typeId_](nmpt<engine::assets::Asset> entry_) {
			return entry_->getTypeId() == typeId_;
		}
	);

	return subrange.end();
}

AssetRegistryBrowserProvider::data_list_type::iterator AssetRegistryBrowserProvider::filterByTypes(
	ref<const data_list_type::iterator> begin_,
	ref<const data_list_type::iterator> end_,
	ref<const CompactSet<AssetTypeId>> types_
) const {

	const auto subrange = std::ranges::remove_if(
		begin_,
		end_,
		[types = &types_](nmpt<engine::assets::Asset> entry_) {
			return types->contains(entry_->getTypeId());
		}
	);

	return subrange.end();
}

void AssetRegistryBrowserProvider::mapAssetsToEntries(
	ref<const data_list_type::iterator> begin_,
	ref<const data_list_type::iterator> end_,
	ref<Vector<AssetBrowserEntry>> entries_
) const {

	assert(begin_ <= end_);

	for (auto it = begin_; it != end_; ++it) {

		entries_.emplace_back(
			AssetBrowserEntryType::eAsset,
			string { (*it)->getAssetName() },
			fs::Url { ""sv, (*it)->getVirtualUrl() },
			(*it)->get_guid()
		);
	}
}

bool AssetRegistryBrowserProvider::effects(ref<const fs::Url> url_) const {
	return true;
}

bool AssetRegistryBrowserProvider::fetchItems(ref<const fs::Url> url_, ref<Vector<AssetBrowserEntry>> entries_) const {

	data_list_type assetList {};
	_registry->findAssetsByPath(url_.path(), assetList);

	/* Map assets to browser entries */

	mapAssetsToEntries(assetList.begin(), assetList.end(), entries_);
	return true;
}

bool AssetRegistryBrowserProvider::fetchDirectories(
	ref<const fs::Url> url_,
	ref<Vector<AssetBrowserEntry>> directories_
) const {

	CompactSet<string> indexedPaths {};
	static_cast<ptr<engine::assets::AssetRegistry>>(_registry.get())->getIndexedPaths(indexedPaths);

	const auto base = url_.path();
	for (ref<const string> entry : indexedPaths) {

		auto check = fs::Path(string_view { entry });
		if (not base.contains(check)) {
			continue;
		}

		if (base != check.pop()) {
			continue;
		}

		directories_.emplace_back(
			AssetBrowserEntryType::eDirectory,
			fs::Path { entry }.name(),
			fs::Url { fs::Path { entry } }
		);
	}

	return true;
}

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
			engine::assets::AssetUrl { (*it)->getAssetVfsUrl() },
			(*it)->getAssetGuid()
		);
	}
}

bool AssetRegistryBrowserProvider::effects(ref<const fs::Url> url_) const {
	return true;
}

bool AssetRegistryBrowserProvider::fetchItems(ref<const fs::Url> url_, ref<Vector<AssetBrowserEntry>> entries_) const {

	auto assetList = data_list_type {};
	_registry->findAssetsByPath(
		engine::assets::AssetPath { url_.path() },
		assetList
	);

	/* Map assets to browser entries */

	mapAssetsToEntries(assetList.begin(), assetList.end(), entries_);
	return true;
}

bool AssetRegistryBrowserProvider::fetchDirectories(
	ref<const fs::Url> url_,
	ref<Vector<AssetBrowserEntry>> directories_
) const {

	auto indexedPaths = Vector<engine::assets::AssetPath> {};
	static_cast<ptr<engine::assets::AssetRegistry>>(_registry.get())->getIndexedPaths(indexedPaths);

	const auto exists = [&directories_](ref<const engine::assets::AssetPath> path_) -> bool {
		return std::ranges::any_of(
			directories_,
			[&path_](const AssetBrowserEntry& entry_) {
				return entry_.url.getAssetPath() == path_;
			}
		);
	};

	const auto base = engine::assets::AssetPath { url_.path() };
	for (const auto& entry : indexedPaths) {

		if (not entry.contains(base)) {
			continue;
		}

		auto asFsPath = fs::Path { String { entry.asByteSpan().data(), entry.asByteSpan().size() } }.normalized();
		while (asFsPath.parentPath() != url_.path() && asFsPath.native().size() > url_.path().native().size()) {
			asFsPath = asFsPath.parentPath();
		}

		auto candidate = engine::assets::AssetPath { asFsPath };
		if (asFsPath.parentPath() == url_.path() && not exists(candidate)) {
			directories_.emplace_back(
				AssetBrowserEntryType::eDirectory,
				engine::assets::AssetUrl { ::hg::move(candidate), engine::assets::AssetName {} }
			);
		}
	}

	return true;
}

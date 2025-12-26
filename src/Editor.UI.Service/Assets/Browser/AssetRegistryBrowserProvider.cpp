#include "AssetRegistryBrowserProvider.hpp"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <ranges>
#include <Engine.Assets.System/AssetRegistry.hpp>
#include <Engine.Assets.Type/Asset.hpp>

#include "AssetBrowserFilter.hpp"
#include "AssetBrowserFilterEntry.hpp"

using namespace hg::editor::ui::service;
using namespace hg;

AssetRegistryBrowserProvider::AssetRegistryBrowserProvider(
	nmpt<engine::assets::IAssetRegistry> registry_
) noexcept :
	_registry(registry_) {}

AssetRegistryBrowserProvider::~AssetRegistryBrowserProvider() = default;

std::ranges::subrange<AssetRegistryBrowserProvider::data_list_type::iterator> AssetRegistryBrowserProvider::filterByTypeAndTag(
	std::ranges::subrange<data_list_type::iterator> range_,
	std::span<const AssetBrowserFilterEntry> filters_
) const {
	const auto removed = std::ranges::remove_if(
		range_,
		[filters_](nmpt<engine::assets::Asset> entry_) {
			return not
			std::ranges::any_of(
				filters_,
				[&entry_](ref<const AssetBrowserFilterEntry> filter_) {
					if (not std::ranges::contains(filter_.types, entry_->getTypeId())) {
						return false;
					}
					// TODO: Check asset tags
					return true;
				}
			);
		}
	);
	return std::ranges::subrange { range_.begin(), removed.begin() };
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
			// TODO: Migrate `fs::Url { ""sv, (*it)->getAssetStorageUrl() }` to enable support for browser paths
			engine::assets::AssetUrl { (*it)->getAssetVfsUrl() },
			(*it)->getAssetGuid()
		);
	}
}

bool AssetRegistryBrowserProvider::effects(ref<const AssetBrowserFilter> filter_) const {
	return true;
}

bool AssetRegistryBrowserProvider::fetchItems(ref<const AssetBrowserFilter> filter_, ref<Vector<AssetBrowserEntry>> entries_) const {

	// TODO: How do we find assets based on the path. The current `findAssetsByPath` will only return assets with exact AssetUrl matches
	// TODO:	Maybe we need to set the `recursive` option within the registry?
	auto assetList = data_list_type {};
	_registry->findAssetsByPath(
		engine::assets::AssetPath { filter_.url.path() },
		assetList
	);

	auto range = std::ranges::subrange { assetList };
	if (not
		filter_.typeAndTag.empty()
	) {
		range = filterByTypeAndTag(range, filter_.typeAndTag);
	}

	/* Map assets to browser entries */

	mapAssetsToEntries(range.begin(), range.end(), entries_);
	return true;
}

bool AssetRegistryBrowserProvider::fetchDirectories(
	ref<const AssetBrowserFilter> filter_,
	ref<Vector<AssetBrowserEntry>> directories_
) const {

	auto indexedPaths = Vector<engine::assets::AssetPath> {};
	static_cast<ptr<engine::assets::AssetRegistry>>(_registry.get())->getIndexedPaths(indexedPaths);

	// TODO: May change vector of directories to a sorted list (insertion-sort like) to reduce runtime-complexity
	// TODO:	O(n^2) > O(lg2(n)) | But this will drastically drop the insert performance due to element shifting.
	const auto exists = [&directories_](ref<const engine::assets::AssetPath> path_) -> bool {
		return std::ranges::any_of(
			directories_,
			[&path_](const AssetBrowserEntry& entry_) {
				return entry_.url.getAssetPath() == path_;
			}
		);
	};

	const auto base = engine::assets::AssetPath { filter_.url.path() };
	for (const auto& entry : indexedPaths) {

		if (not
			entry.contains(base)
		) {
			continue;
		}

		auto asFsPath = fs::Path { String { entry.asByteSpan().data(), entry.asByteSpan().size() } }.normalized();
		while (asFsPath.parentPath() != filter_.url.path() && asFsPath.native().size() > filter_.url.path().native().size()) {
			// Error: Infinite loop if entry url is not strictly rebase/sanitized as virtual fs-path
			auto next = asFsPath.parentPath();
			if (next == asFsPath) {
				break;
			}

			asFsPath = ::hg::move(next);
		}

		auto candidate = engine::assets::AssetPath { asFsPath };
		if (asFsPath.parentPath() == filter_.url.path() && not
			exists(candidate)
		) {
			directories_.emplace_back(
				AssetBrowserEntryType::eDirectory,
				engine::assets::AssetUrl { ::hg::move(candidate), engine::assets::AssetName {} }
			);
		}
	}

	return true;
}
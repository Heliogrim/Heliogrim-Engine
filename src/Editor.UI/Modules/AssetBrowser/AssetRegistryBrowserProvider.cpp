#include "AssetRegistryBrowserProvider.hpp"

#include <algorithm>
#include <Engine.Assets/Types/Asset.hpp>

#include <Engine.Assets.System/AssetRegistry.hpp>
#include <filesystem>

using namespace hg::editor::ui;
using namespace hg;

AssetRegistryBrowserProvider::AssetRegistryBrowserProvider(
    const non_owning_rptr<engine::assets::IAssetRegistry> registry_
) noexcept :
    _registry(registry_) {}

AssetRegistryBrowserProvider::~AssetRegistryBrowserProvider() = default;

AssetRegistryBrowserProvider::data_list_type::iterator AssetRegistryBrowserProvider::filterByType(
    cref<data_list_type::iterator> begin_,
    cref<data_list_type::iterator> end_,
    const asset_type_id typeId_
) const {

    const auto subrange = _STD ranges::remove_if(
        begin_,
        end_,
        [typeId_](ptr<engine::assets::Asset> entry_) {
            return entry_->getTypeId() == typeId_;
        }
    );

    return subrange.end();
}

AssetRegistryBrowserProvider::data_list_type::iterator AssetRegistryBrowserProvider::filterByTypes(
    cref<data_list_type::iterator> begin_,
    cref<data_list_type::iterator> end_,
    cref<CompactSet<asset_type_id>> types_
) const {

    const auto subrange = _STD ranges::remove_if(
        begin_,
        end_,
        [types = &types_](ptr<engine::assets::Asset> entry_) {
            return types->contains(entry_->getTypeId());
        }
    );

    return subrange.end();
}

void AssetRegistryBrowserProvider::mapAssetsToEntries(
    cref<data_list_type::iterator> begin_,
    cref<data_list_type::iterator> end_,
    ref<Vector<AssetBrowserEntry>> entries_
) const {

    assert(begin_ <= end_);

    for (auto iter = begin_; iter != end_; ++iter) {

        AssetBrowserEntry entry {
            .type = AssetBrowserEntryType::eAsset,
            .title = string { (*iter)->getAssetName() },
            .path = fs::Url { ""sv, (*iter)->getVirtualUrl() },
            .guid = (*iter)->get_guid()
        };

        entries_.push_back(_STD move(entry));
    }
}

bool AssetRegistryBrowserProvider::effects(cref<fs::Url> url_) {
    return true;
}

bool AssetRegistryBrowserProvider::retrieve(cref<fs::Url> url_, ref<Vector<AssetBrowserEntry>> entries_) {

    data_list_type assetList {};
    _registry->findAssetsByPath(url_.path(), assetList);

    /* Map assets to browser entries */

    mapAssetsToEntries(assetList.begin(), assetList.end(), entries_);
    return true;
}

bool AssetRegistryBrowserProvider::retrieveDirectories(
    cref<fs::Url> url_,
    ref<Vector<AssetBrowserEntry>> directories_
) {

    CompactSet<string> indexedPaths {};
    static_cast<ptr<engine::assets::AssetRegistry>>(_registry)->getIndexedPaths(indexedPaths);

    const _STD filesystem::path base { url_.path() };
    for (auto&& entry : indexedPaths) {

        const _STD filesystem::path check { _STD move(entry) };
        const auto rel = _STD filesystem::relative(check, base);

        if (rel.has_parent_path()) {
            continue;
        }

        directories_.push_back(
            AssetBrowserEntry {
                .type = AssetBrowserEntryType::eDirectory,
                .title = check.filename().string(),
                .path = fs::Url { ""sv, check.string() }
            }
        );
    }

    return true;
}

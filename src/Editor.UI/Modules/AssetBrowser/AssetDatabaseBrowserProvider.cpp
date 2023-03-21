#include "AssetDatabaseBrowserProvider.hpp"

#include <Engine.Assets/Types/Asset.hpp>

using namespace hg::editor::ui;
using namespace hg;

AssetDatabaseBrowserProvider::AssetDatabaseBrowserProvider(
    const non_owning_rptr<engine::assets::IAssetRegistry> database_
) noexcept :
    _registry(database_) {}

AssetDatabaseBrowserProvider::~AssetDatabaseBrowserProvider() = default;

non_owning_rptr<AssetDatabaseBrowserProvider::this_type> AssetDatabaseBrowserProvider::createProvider(cref<Url> url_) {
    return this;
}

non_owning_rptr<AssetDatabaseBrowserProvider::this_type> AssetDatabaseBrowserProvider::destroyProvider(cref<Url> url_) {
    return this;
}

AssetDatabaseBrowserProvider::data_list_type::iterator AssetDatabaseBrowserProvider::filterByType(
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

AssetDatabaseBrowserProvider::data_list_type::iterator AssetDatabaseBrowserProvider::filterByTypes(
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

void AssetDatabaseBrowserProvider::mapAssetsToEntries(
    cref<data_list_type::iterator> begin_,
    cref<data_list_type::iterator> end_,
    ref<Vector<AssetBrowserEntry>> entries_
) const {

    assert(begin_ <= end_);

    #if FALSE
    for (auto iter = begin_; iter != end_; ++iter) {

        AssetBrowserEntry entry {
            .url = Url {},
            .asset = {
                .type = AssetBrowserEntryType::eAsset,
                .typeId = (*iter)->getTypeId(),
                .guid = (*iter)->get_guid()
            }
        };

        // Warning: We might not copy nor move AssetBrowserEntry, cause union types are non-trivial
        entries_.push_back(_STD move(entry));
    }
    #endif
}

bool AssetDatabaseBrowserProvider::retrieve(cref<Url> url_, ref<Vector<AssetBrowserEntry>> entries_) {

    /* Map urls to target types */

    CompactSet<asset_type_id> types {};

    /* Retrieve assets from database with url */

    // Warning: Currently we don't have a possibility to iterator the database, nor filter or index them by url
    // TODO: We need a editor helper to capture/index loaded assets and there related paths
    _registry;
    data_list_type assetList {};

    const auto first = assetList.begin();
    auto end = assetList.end();

    /**/

    end = filterByTypes(first, end, types);

    /* Map assets to browser entries */

    mapAssetsToEntries(first, end, entries_);
    return true;
}

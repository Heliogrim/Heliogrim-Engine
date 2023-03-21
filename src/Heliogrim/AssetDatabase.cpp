#include "AssetDatabase.hpp"

#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets.System/AssetDescriptor.hpp>
#include <Engine.Assets/Types/Asset.hpp>
#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Core/Engine.hpp>

using namespace hg;

AssetDatabase::AssetDatabase(const non_owning_rptr<void> internal_) :
    _internal(internal_) {}

AssetDatabase::~AssetDatabase() = default;

bool AssetDatabase::contains(cref<asset_guid> guid_) const noexcept {

    const auto& idb { *static_cast<const non_owning_rptr<engine::assets::IAssetRegistry>>(_internal) };
    return idb.hasAsset(guid_);
}

AssetDatabaseResult<Asset> AssetDatabase::operator[](cref<asset_guid> guid_) const {

    const auto& idb { *static_cast<const non_owning_rptr<engine::assets::IAssetRegistry>>(_internal) };
    auto* const asset = idb.findAssetByGuid(guid_);

    if (asset == nullptr) {
        return AssetDatabaseResult<Asset> {
            { AssetDatabaseResultType::eFailed },
            hg::Asset { invalid_asset_guid, asset_type_id { 0 }, nullptr }
        };
    }

    return AssetDatabaseResult<Asset> {
        { AssetDatabaseResultType::eSuccess },
        {
            asset->get_guid(),
            asset->getTypeId(),
            asset
        }
    };
}

bool AssetDatabase::insert(ptr<Asset> asset_) noexcept {

    DEBUG_ASSERT(asset_->_internal != nullptr, "Asset should have internal state representation.")

    auto& idb { *static_cast<const non_owning_rptr<engine::assets::IAssetRegistry>>(_internal) };
    engine::assets::storeDefaultNameAndUrl(static_cast<ptr<engine::assets::Asset>>(asset_->_internal), {});
    return idb.insert({ static_cast<ptr<engine::assets::Asset>>(asset_->_internal) });
}

bool AssetDatabase::erase(ptr<Asset> asset_) noexcept {
    auto& idb { *static_cast<const non_owning_rptr<engine::assets::IAssetRegistry>>(_internal) };
    return idb.removeAssetByGuid(asset_->guid());
}

#include "AssetDatabase.hpp"

#include <Engine.Assets/Database/AssetDatabaseQuery.hpp>

using namespace ember;

AssetDatabase::AssetDatabase(managed<void> internal_) :
    _internal(internal_) {}

AssetDatabase::~AssetDatabase() = default;

bool AssetDatabase::contains(cref<asset_guid> guid_) const noexcept {

    auto& idb { *static_cast<ptr<engine::assets::AssetDatabase>>(_internal.get()) };
    auto query { idb.query(guid_) };

    return query.exists();
}

AssetDatabaseResult<> AssetDatabase::operator[](cref<asset_guid> guid_) const {

    auto& idb { *static_cast<ptr<engine::assets::AssetDatabase>>(_internal.get()) };
    auto query { idb.query(guid_) };

    if (!query.exists()) {
        return AssetDatabaseResult<> { { AssetDatabaseResultType::eFailed }, nullptr };
    }

    return AssetDatabaseResult<> {
        { AssetDatabaseResultType::eSuccess },
        query.get()
    };
}

bool AssetDatabase::insert(ptr<Asset> asset_) noexcept {

    DEBUG_ASSERT(asset_->_internal != nullptr, "Asset should have internal state representation.");

    auto& idb { *static_cast<ptr<engine::assets::AssetDatabase>>(_internal.get()) };
    auto query { idb.query(asset_->guid()) };

    return query.insert(asset_->typeId(), static_cast<ptr<engine::assets::Asset>>(asset_->_internal));
}

bool AssetDatabase::erase(ptr<Asset> asset_) noexcept {

    auto& idb { *static_cast<ptr<engine::assets::AssetDatabase>>(_internal.get()) };
    auto query { idb.query(asset_->guid()) };

    return query.remove();
}

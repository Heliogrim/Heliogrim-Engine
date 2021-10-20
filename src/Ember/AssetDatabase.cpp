#include "AssetDatabase.hpp"

#include <Engine.Assets/Database/AssetDatabaseQuery.hpp>
#include <Engine.Assets/Database/AssetSeeder.hpp>
#include <Engine.Session/Session.hpp>

using namespace ember;

AssetDatabase::AssetDatabase(const ptr<void> internal_) :
    _internal(internal_) {}

AssetDatabase::~AssetDatabase() = default;

bool AssetDatabase::contains(cref<asset_guid> guid_) const noexcept {

    auto& idb { *static_cast<const ptr<engine::assets::AssetDatabase>>(_internal) };
    auto query { idb.query(guid_) };

    return query.exists();
}

AssetDatabaseResult<Asset> AssetDatabase::operator[](cref<asset_guid> guid_) const {

    auto& idb { *static_cast<const ptr<engine::assets::AssetDatabase>>(_internal) };
    auto query { idb.query(guid_) };

    if (!query.exists()) {
        return AssetDatabaseResult<Asset> {
            { AssetDatabaseResultType::eFailed },
            ember::Asset { invalid_asset_guid, asset_type_id { 0 }, nullptr }
        };
    }

    auto value = query.get();
    return AssetDatabaseResult<Asset> {
        { AssetDatabaseResultType::eSuccess },
        {
            value->get_guid(),
            value->get_typeId(),
            value
        }
    };
}

bool AssetDatabase::insert(ptr<Asset> asset_) noexcept {

    DEBUG_ASSERT(asset_->_internal != nullptr, "Asset should have internal state representation.")

    auto& idb { *static_cast<const ptr<engine::assets::AssetDatabase>>(_internal) };
    auto query { idb.query(asset_->guid()) };

    return query.insert(asset_->typeId(), static_cast<ptr<engine::assets::Asset>>(asset_->_internal));
}

bool AssetDatabase::autoRegister(ptr<ember::Asset> (* fncPtr_)()) noexcept {

    DEBUG_ASSERT(fncPtr_ != nullptr, "Give function pointer (constructor) should not be nullptr.")

    auto* seeder { engine::Session::get()->assetSeeder() };
    DEBUG_ASSERT(seeder != nullptr, "Asset Seeder should be present while accessing auto insertation.")

    return seeder->autoRegister(fncPtr_);
}

bool AssetDatabase::erase(ptr<Asset> asset_) noexcept {

    auto& idb { *static_cast<const ptr<engine::assets::AssetDatabase>>(_internal) };
    auto query { idb.query(asset_->guid()) };

    return query.remove();
}

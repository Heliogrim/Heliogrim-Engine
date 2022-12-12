#include "AssetDatabase.hpp"

#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets/Database/AssetDatabase.hpp>
#include <Engine.Assets/Database/AssetDatabaseQuery.hpp>
#include <Engine.Assets/Database/AssetSeeder.hpp>
#include <Engine.Core/Engine.hpp>

using namespace ember;

AssetDatabase::AssetDatabase(const non_owning_rptr<void> internal_) :
    _internal(internal_) {}

AssetDatabase::~AssetDatabase() = default;

bool AssetDatabase::contains(cref<asset_guid> guid_) const noexcept {

    const auto& idb { *static_cast<const non_owning_rptr<engine::assets::AssetDatabase>>(_internal) };
    const auto query { idb.query(guid_) };

    return query.exists();
}

AssetDatabaseResult<Asset> AssetDatabase::operator[](cref<asset_guid> guid_) const {

    const auto& idb { *static_cast<const non_owning_rptr<engine::assets::AssetDatabase>>(_internal) };
    const auto query { idb.query(guid_) };

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
            value->getTypeId(),
            value
        }
    };
}

bool AssetDatabase::insert(ptr<Asset> asset_) noexcept {

    DEBUG_ASSERT(asset_->_internal != nullptr, "Asset should have internal state representation.")

    const auto& idb { *static_cast<const non_owning_rptr<engine::assets::AssetDatabase>>(_internal) };
    auto query { idb.query(asset_->guid()) };

    return query.insert(asset_->typeId(), static_cast<ptr<engine::assets::Asset>>(asset_->_internal));
}

bool AssetDatabase::autoRegister(ptr<ember::Asset> (* fncPtr_)()) noexcept {

    DEBUG_ASSERT(fncPtr_ != nullptr, "Give function pointer (constructor) should not be nullptr.")

    const auto* const assets { engine::Engine::getEngine()->getAssets() };
    auto* seeder { assets->getSeeder() };
    DEBUG_ASSERT(seeder != nullptr, "Asset Seeder should be present while accessing auto insertation.")

    return seeder->autoRegister(fncPtr_);
}

bool AssetDatabase::erase(ptr<Asset> asset_) noexcept {

    const auto& idb { *static_cast<const non_owning_rptr<engine::assets::AssetDatabase>>(_internal) };
    auto query { idb.query(asset_->guid()) };

    return query.remove();
}

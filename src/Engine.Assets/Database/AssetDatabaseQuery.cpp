#include "AssetDatabaseQuery.hpp"

#include "AssetDatabase.hpp"

using namespace ember::engine::assets;
using namespace ember;

const ptr<AssetDatabase> AssetDatabaseQuery::database() const noexcept {
    return _database;
}

ptr<AssetDatabase> AssetDatabaseQuery::database() noexcept {
    return _database;
}

bool AssetDatabaseQuery::exists() const noexcept {
    return _database->has(_guid);
}

ptr<Asset> AssetDatabaseQuery::get() const {
    return (*_database)[_guid];
}

ptr<Asset> AssetDatabaseQuery::get(nothrow_t) const noexcept {
    if (_database->has(_guid)) {
        return get();
    }
    return nullptr;
}

bool AssetDatabaseQuery::insert(cref<asset_type_id> type_, ptr<Asset> asset_) noexcept {
    return _database->insert(_guid, type_, asset_);
}

ptr<Asset> AssetDatabaseQuery::remove() noexcept {
    return _database->remove(_guid);
}

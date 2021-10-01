#include "AssetDatabase.hpp"

#include "AssetDatabaseQuery.hpp"

using namespace ember::engine::assets;
using namespace ember;

AssetDatabase::AssetDatabase() noexcept = default;

AssetDatabase::~AssetDatabase() noexcept {

    SCOPED_STOPWATCH
    _SCTRL_GATE(_mtx);

    tidy();
}

void AssetDatabase::tidy() {}

bool AssetDatabase::has(cref<asset_guid> guid_) const noexcept {

    SCOPED_STOPWATCH
    _SCTRL_SGATE(_mtx);

    const auto pos = _mapping.find({ guid_ });
    return pos != _mapping.cend();
}

ptr<Asset> AssetDatabase::operator[](cref<asset_guid> guid_) const {

    SCOPED_STOPWATCH
    _SCTRL_SGATE(_mtx);

    const auto pos = _mapping.find({ guid_ });
    if (pos == _mapping.cend()) {
        throw _STD runtime_error(R"(Can not find AssetDatabaseEntry by given asset_guid.)");
    }
    return pos.value().asset();
}

bool AssetDatabase::insert(cref<asset_guid> guid_, const ptr<Asset> asset_) noexcept {

    SCOPED_STOPWATCH
    _SCTRL_GATE(_mtx);

    AssetDatabaseEntry entry { guid_, asset_ };
    const auto pos = _mapping.insert_or_assign(_STD move(entry));

    // TODO: Rewrite
    return true;
}

ptr<Asset> AssetDatabase::remove(cref<asset_guid> guid_) noexcept {

    SCOPED_STOPWATCH
    _SCTRL_GATE(_mtx);

    const auto pos = _mapping.find({ guid_ });

    if (pos == _mapping.end()) {
        return nullptr;
    }

    const auto old { _STD move(pos.value()) };
    _mapping.erase(pos);

    return old.asset();
}

AssetDatabaseQuery AssetDatabase::query(cref<asset_guid> guid_) const noexcept {
    return AssetDatabaseQuery { const_cast<ptr<AssetDatabase>>(this), guid_ };
}

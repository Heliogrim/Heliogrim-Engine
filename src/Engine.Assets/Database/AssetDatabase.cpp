#include "AssetDatabase.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "AssetDatabaseQuery.hpp"
#include "../AssetFactory.hpp"

using namespace ember::engine::assets;
using namespace ember;

AssetDatabase::AssetDatabase() noexcept :
    _mapping() {}

AssetDatabase::~AssetDatabase() noexcept {
    tidy();
}

void AssetDatabase::tidy() {

    SCOPED_STOPWATCH
    _SCTRL_GATE(_mtx);

    for (auto& entry : _mapping) {
        delete const_cast<ptr<Asset>>(entry.asset());
    }

    _mapping.clear();

}

bool AssetDatabase::has(cref<asset_guid> guid_) const noexcept {

    SCOPED_STOPWATCH
    _SCTRL_SGATE(_mtx);

    const auto pos = _mapping.find({ guid_, asset_type_id { 0 }, nullptr });
    return pos != _mapping.cend();
}

ptr<Asset> AssetDatabase::operator[](cref<asset_guid> guid_) const {

    SCOPED_STOPWATCH
    _SCTRL_SGATE(_mtx);

    const auto pos = _mapping.find({ guid_, asset_type_id { 0 } });
    if (pos == _mapping.cend()) {
        throw _STD runtime_error(R"(Can not find AssetDatabaseEntry by given asset_guid.)");
    }
    return (*pos).asset();
}

bool AssetDatabase::insert(cref<asset_guid> guid_, cref<asset_type_id> type_, const ptr<Asset> asset_) noexcept {

    SCOPED_STOPWATCH
    _SCTRL_GATE(_mtx);

    AssetDatabaseEntry entry { guid_, type_, asset_ };
    const auto pos = _mapping.insert(_STD move(entry));

    auto s = _mapping.size();
    auto b = _mapping.bucket_count();
    auto ml = _mapping.max_load_factor();
    auto l = _mapping.load_factor();
    auto ms = _mapping.max_size();
    auto mb = _mapping.max_bucket_count();

    // TODO: Rewrite
    return pos.second;
}

ptr<Asset> AssetDatabase::remove(cref<asset_guid> guid_) noexcept {

    SCOPED_STOPWATCH
    _SCTRL_GATE(_mtx);

    auto pos = _mapping.find({ guid_, asset_type_id { 0 }, nullptr });

    if (pos == _mapping.end()) {
        return nullptr;
    }

    const auto old { _STD move(*pos) };
    _mapping.erase(pos);

    return old.asset();
}

AssetDatabaseQuery AssetDatabase::query(cref<asset_guid> guid_) const noexcept {
    return AssetDatabaseQuery { const_cast<ptr<AssetDatabase>>(this), guid_ };
}

#include "AssetGuid.hpp"

#include <atomic>
#include <chrono>

#include <Engine.Common/Guid.hpp>

using namespace hg;

asset_guid hg::generate_unstable_asset_guid() {
    static std::atomic_uint_fast32_t ai { 1 };

    const auto pre = ai.fetch_add(1, std::memory_order_acq_rel);

    // TODO: Replace temporary solution
    const auto tt = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    return { pre, 0, 0, tt };
}

asset_guid hg::generate_asset_guid() {

    asset_guid guid {};
    GuidGenerate(guid);

    return guid;
}

#include "AssetGuid.hpp"

#include <atomic>
#include <chrono>

using namespace ember;

asset_guid ember::generate_asset_guid() {
    static _STD atomic_uint_fast32_t ai { 1 };

    const auto pre = ai.fetch_add(1, _STD memory_order_acq_rel);

    // TODO: Replace temporary solution
    const auto tt = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    return { pre, 0, 0, tt };
}

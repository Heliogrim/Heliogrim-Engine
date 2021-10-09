#include "AssetGuid.hpp"

using namespace ember;

asset_guid ember::generate_asset_guid() {
    static _STD atomic_uint_fast32_t ai { 1 };
    const auto post = ai.fetch_add(1, _STD memory_order_relaxed);
    return { 0, 0, 0, post };
}

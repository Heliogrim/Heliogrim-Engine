#pragma once

#include "__fwd.hpp"
#include "LoaderStage.hpp"
#include "CacheRequest.hpp"
#include "CacheResponse.hpp"

namespace ember::engine::resource::loader {
    template <typename AssetType_, typename ResourceType_>
    class Cache :
        public CacheStage<AssetType_, ResourceType_> {
    public:
        using this_type = Cache<AssetType_, ResourceType_>;
        using underlying_type = CacheStage<AssetType_, ResourceType_>;

        using underlying_type::traits;

        using sub_request_type = typename underlying_type::traits::request_type;
        using sub_response_type = typename underlying_type::traits::response_type;
    };
}

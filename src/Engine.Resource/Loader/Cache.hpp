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
        using underlying_type = CacheStage<AssetType_, ResourceType_>;

        using underlying_type::response_value_type;
        using underlying_type::request_value_type;
        using underlying_type::request_options_type;
    };
}

#pragma once

#include "__fwd.hpp"
#include "LoaderStage.hpp"

namespace ember::engine::resource::loader {
    template <typename AssetType_, typename ResourceType_>
    class Transformer :
        public TransformerStage<AssetType_, ResourceType_> {
    public:
        using underlying_type = TransformerStage<AssetType_, ResourceType_>;

        using underlying_type::response_value_type;
        using underlying_type::request_value_type;
        using underlying_type::request_options_type;
    };
}

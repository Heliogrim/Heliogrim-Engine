#pragma once

#include "__fwd.hpp"
#include "LoaderStage.hpp"
#include "TransformerRequest.hpp"
#include "TransformerResponse.hpp"

namespace ember::engine::resource::loader {
    template <typename AssetType_, typename ResourceType_>
    class Transformer :
        public TransformerStage<AssetType_, ResourceType_> {
    public:
        using underlying_type = TransformerStage<AssetType_, ResourceType_>;

        using underlying_type::traits;

        using sub_request_type = void;
        using sub_response_type = void;
    };
}

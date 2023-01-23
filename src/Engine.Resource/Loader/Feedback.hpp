#pragma once

#include "__fwd.hpp"
#include "LoaderStage.hpp"
#include "FeedbackRequest.hpp"
#include "FeedbackResponse.hpp"

namespace ember::engine::resource::loader {
    template <typename AssetType_, typename ResourceType_>
    class Feedback :
        public FeedbackStage<AssetType_, ResourceType_> {
    public:
        using underlying_type = FeedbackStage<AssetType_, ResourceType_>;

        using underlying_type::response_value_type;
        using underlying_type::request_value_type;
        using underlying_type::request_options_type;
    };
}

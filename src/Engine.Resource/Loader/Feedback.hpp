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
        using this_type = Feedback<AssetType_, ResourceType_>;
        using underlying_type = FeedbackStage<AssetType_, ResourceType_>;

        using underlying_type::traits;

        using sub_request_type = typename underlying_type::traits::request_type;
        using sub_response_type = typename underlying_type::traits::response_type;
    };
}

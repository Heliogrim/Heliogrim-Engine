#pragma once

#include "__fwd.hpp"

namespace ember::engine::resource::loader {
    template <IsRequestValueType ValueType_>
    struct TransformerRequest {
        using this_type = TransformerRequest<ValueType_>;

        using value_type = ValueType_;
        using options_type = RequestOptions<this_type, ValueType_>;

        using stream_type = _STD conditional_t<
            assets::IsStreamableAsset<ValueType_>,
            StreamOptions<this_type, ValueType_>,
            void>;
    };
}

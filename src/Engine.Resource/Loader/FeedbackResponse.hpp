#pragma once

#include "__fwd.hpp"

namespace ember::engine::resource::loader {
    template <IsResponseValueType ValueType_>
    struct FeedbackResponse {
        template <typename Type_>
        using wrapper_type = smr<Type_>;
        using value_type = ValueType_;

        using type = smr<value_type>;
    };
}

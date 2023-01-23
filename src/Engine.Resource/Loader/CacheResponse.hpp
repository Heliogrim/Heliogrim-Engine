#pragma once

#include "__fwd.hpp"

namespace ember::engine::resource::loader {
    template <IsResponseValueType ValueType_>
    struct CacheResponse {
        using value_type = ValueType_;
    };
}

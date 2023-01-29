#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "__fwd.hpp"

namespace ember::engine::resource::loader {
    template <IsResponseValueType ValueType_>
    struct CacheResponse {
        template <typename Type_>
        using wrapper_type = smr<Type_>;
        using value_type = ValueType_;

        using type = smr<value_type>;
    };
}

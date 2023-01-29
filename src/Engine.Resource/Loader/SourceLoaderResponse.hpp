#pragma once

#include "__fwd.hpp"

#include "../Source/Source.hpp"

namespace ember::engine::resource::loader {
    template <IsResponseValueType ValueType_>
    struct SourceLoaderResponse {
        //using value_type = ValueType_;
        static_assert(_STD is_void_v<ValueType_>);

        template <typename Type_>
        using wrapper_type = smr<Type_>;
        using value_type = Resource<Source>;

        using type = smr<value_type>;
    };
}

#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"

namespace ember::engine::resource::loader {
    template <IsRequestValueType ValueType_>
    struct LoaderRequest {
        using this_type = LoaderRequest<ValueType_>;

        template <typename Type_>
        using wrapper_type = non_owning_rptr<Type_>;
        using value_type = ValueType_;

        using type = wrapper_type<value_type>;

        /**/

        template <typename Type_>
        using options_wrapper_type = _STD type_identity_t<Type_>;
        using options_type = typename RequestOptions<this_type>::type;

        using options = options_wrapper_type<options_type>;

        /**/

        template <typename Type_>
        using stream_wrapper_type = _STD type_identity_t<Type_>;
        using stream_type = typename StreamOptions<this_type>::type;

        using stream = _STD conditional_t<
            assets::IsStreamableAsset<ValueType_>,
            stream_wrapper_type<stream_type>,
            void>;
    };

    template <IsResponseValueType ValueType_>
    struct LoaderResponse {
        using this_type = LoaderResponse<ValueType_>;

        template <typename Type_>
        using wrapper_type = smr<Type_>;
        using value_type = ValueType_;

        using type = wrapper_type<value_type>;
    };

    template <typename RequestValueType_, typename ResponseValueType_>
    struct LoaderTraits {
        using request = LoaderRequest<RequestValueType_>;
        using response = LoaderResponse<ResponseValueType_>;
    };
}

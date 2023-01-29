#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "SourceLoaderRequest.hpp"
#include "SourceLoaderResponse.hpp"
#include "LoaderStage.hpp"

#include "__fwd.hpp"

namespace ember::engine::resource::loader {
    /*
    template <>
    struct SourceLoaderRequest<void> {};

    template <>
    struct SourceLoaderResponse<void> {};
     */

    template <>
    struct RequestOptions<SourceLoaderRequest<void>, void> {};
}

namespace ember::engine::resource::loader {
    class SourceLoader :
        public SourceLoaderStage<> {
    public:
        using this_type = SourceLoader;
        using underlying_type = SourceLoaderStage<>;

        using underlying_type::traits;

    public:
        [[nodiscard]] traits::response_value_type operator()(
            mref<traits::request_value_type> request_,
            mref<traits::request_options_type> options_
        ) const override;
    };
}

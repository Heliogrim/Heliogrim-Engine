#pragma once

#include "LoaderStage.hpp"

#include "__fwd.hpp"

namespace ember::engine::resource::loader {
    class SourceLoader :
        public SourceLoaderStage {
    public:
        using underlying_type = SourceLoaderStage;

        using underlying_type::response_value_type;
        using underlying_type::request_value_type;
        using underlying_type::request_options_type;

    public:
        [[nodiscard]] response_value_type operator()(
            mref<request_value_type> request_,
            mref<request_options_type> options_
        ) const override;
    };
}

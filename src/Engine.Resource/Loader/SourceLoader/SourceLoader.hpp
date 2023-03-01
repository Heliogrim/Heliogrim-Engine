#pragma once

#include "../SourceLoader.hpp"

namespace hg::engine::resource::loader {
    class SourceLoader :
        public SourceLoaderStage {
    public:
        using this_type = SourceLoader;
        using underlying_type = SourceLoaderStage;

        using underlying_type::request_type;
        using underlying_type::response_type;

        using underlying_type::stream_request_type;
        using underlying_type::stream_response_type;

    public:
        SourceLoader();

        ~SourceLoader() noexcept override;

    public:
        [[nodiscard]] response_type::type operator()(
            mref<request_type::type> request_,
            mref<request_type::options> options_
        ) const override;

        [[nodiscard]] stream_response_type::type operator()(
            mref<stream_request_type::type> request_,
            mref<stream_request_type::options> options_
        ) const override;
    };
}

#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"
#include "SourceLoaderTraits.hpp"

namespace ember::engine::resource::loader {
    class __declspec(novtable) SourceLoaderStage {
    public:
        using this_type = SourceLoaderStage;

        using request_type = SourceLoaderRequest<void>;
        using response_type = SourceLoaderResponse<void>;

        using stream_request_type = SourceLoaderStreamRequest<void>;
        using stream_response_type = SourceLoaderStreamResponse<void>;

    public:
        virtual ~SourceLoaderStage() noexcept = default;

    public:
        [[nodiscard]] virtual response_type::type operator()(
            _In_ mref<request_type::type> request_,
            _In_ mref<request_type::options> options_
        ) const = 0;

        [[nodiscard]] virtual stream_response_type::type operator()(
            _In_ mref<stream_request_type::type> request_,
            _In_ mref<stream_request_type::options> options_
        ) const = 0;
    };

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

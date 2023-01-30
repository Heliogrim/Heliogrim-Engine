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

    public:
        virtual ~SourceLoaderStage() noexcept = default;

    public:
        [[nodiscard]] virtual response_type::type operator()(
            _In_ mref<request_type::type> request_,
            _In_ mref<request_type::options> options_
        ) const = 0;

        [[nodiscard]] virtual response_type::type operator()(
            _In_ mref<request_type::type> request_,
            _In_ mref<request_type::options> options_,
            _In_ mref<request_type::stream> streamOptions_
        ) const = 0;
    };

    class SourceLoader :
        public SourceLoaderStage {
    public:
        using this_type = SourceLoader;
        using underlying_type = SourceLoaderStage;

        using underlying_type::request_type;
        using underlying_type::response_type;

    public:
        ~SourceLoader() noexcept override;

    public:
        [[nodiscard]] response_type::type operator()(
            mref<request_type::type> request_,
            mref<request_type::options> options_
        ) const override;

        [[nodiscard]] response_type::type operator()(
            mref<request_type::type> request_,
            mref<request_type::options> options_,
            mref<request_type::stream> streamOptions_
        ) const override;
    };
}

#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"

namespace ember::engine::resource::loader {
    namespace {
        template <typename LoaderStage_, bool>
        struct StreamLoaderStage;

        template <typename LoaderStage_>
        struct __declspec(novtable) StreamLoaderStage<LoaderStage_, false> {};

        template <typename LoaderStage_>
        struct __declspec(novtable) StreamLoaderStage<LoaderStage_, true> {
        public:
            using loader_stage_type = LoaderStage_;

            using response_value_type = typename loader_stage_type::response_value_type;
            using request_value_type = typename loader_stage_type::request_value_type;
            using request_options_type = typename loader_stage_type::request_options_type;
            using stream_options_type = typename loader_stage_type::stream_options_type;

        public:
            virtual ~StreamLoaderStage() = default;

        public:
            [[nodiscard]] virtual response_value_type operator()(
                mref<request_value_type> request_,
                mref<request_options_type> options_,
                mref<stream_options_type> streamOptions_
            ) const = 0;
        };
    }

    template <IsRequestType RequestType_, IsResponseType ResponseType_>
    class __declspec(novtable) LoaderStage :
        StreamLoaderStage<
            LoaderStage<RequestType_, ResponseType_>,
            assets::IsStreamableAsset<typename RequestType_::value_type>
        > {
    public:
        using request_type = RequestType_;
        using request_value_type = typename request_type::value_type;
        using request_options_type = typename request_type::options_type;

        using response_type = ResponseType_;
        using response_value_type = typename response_type::value_type;

    public:
        virtual ~LoaderStage() override = default;

    public:
        [[nodiscard]] virtual response_value_type operator()(
            mref<request_value_type> request_,
            mref<request_options_type> options_
        ) const = 0;
    };
}

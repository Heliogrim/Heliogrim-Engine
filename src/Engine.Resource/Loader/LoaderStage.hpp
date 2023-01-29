#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Assets/Types/AssetConcept.hpp>

#include "__fwd.hpp"
#include "LoaderStageTraits.hpp"

#include "CacheRequest.hpp"
#include "FeedbackRequest.hpp"
#include "TransformerRequest.hpp"
#include "SourceLoaderRequest.hpp"

#include "CacheResponse.hpp"
#include "FeedbackResponse.hpp"
#include "TransformerResponse.hpp"
#include "SourceLoaderResponse.hpp"

namespace ember::engine::resource::loader {
    namespace {
        template <typename Traits_, bool>
        struct StreamLoaderStage;

        template <typename Traits_>
        struct __declspec(novtable) StreamLoaderStage<Traits_, false> {
        public:
            virtual ~StreamLoaderStage() = default;
        };

        template <typename Traits_>
        struct __declspec(novtable) StreamLoaderStage<Traits_, true> {
        public:
            using traits = Traits_;

        public:
            virtual ~StreamLoaderStage() = default;

        public:
            [[nodiscard]] virtual typename traits::response_value_type operator()(
                mref<typename traits::request_value_type> request_,
                mref<typename traits::request_options_type> options_,
                mref<typename traits::stream_options_type> streamOptions_
            ) const = 0;
        };
    }

    template </*IsRequestType*/typename RequestType_, /*IsResponseType*/typename ResponseType_>
    class __declspec(novtable) LoaderStage :
        StreamLoaderStage<
            LoaderStageTraits<RequestType_, ResponseType_>,
            assets::IsStreamableAsset<typename RequestType_::value_type>
        > {
    public:
        using traits = LoaderStageTraits<RequestType_, ResponseType_>;

    public:
        ~LoaderStage() override = default;

    public:
        [[nodiscard]] virtual typename traits::response_value_type operator()(
            mref<typename traits::request_value_type> request_,
            mref<typename traits::request_options_type> options_
        ) const = 0;
    };
}

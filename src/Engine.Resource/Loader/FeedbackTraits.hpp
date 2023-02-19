#pragma once

#include "__fwd.hpp"
#include "LoaderTraits.hpp"

namespace ember::engine::resource::loader {
    template <typename AssetType_, typename ResourceType_>
    class Feedback;

    /**/

    template <IsRequestValueType AssetType_>
    struct FeedbackRequest {
        using this_type = FeedbackRequest<AssetType_>;

        template <typename Type_>
        using wrapper_type = typename LoaderRequest<AssetType_>::template wrapper_type<Type_>;
        using value_type = AssetType_;

        using type = wrapper_type<value_type>;

        /**/

        template <typename Type_>
        using options_wrapper_type = _STD type_identity_t<Type_>;
        using options_type = typename RequestOptions<this_type>::type;

        using options = options_wrapper_type<options_type>;
    };

    template <IsResponseValueType ResourceType_>
    struct FeedbackResponse {
        using this_type = FeedbackResponse<ResourceType_>;

        template <typename Type_>
        using wrapper_type = typename LoaderResponse<ResourceType_>::template wrapper_type<Type_>;
        using value_type = ResourceType_;

        using type = wrapper_type<value_type>;
    };

    /**/

    template <IsStreamRequestSubjectType SubjectType_>
    struct FeedbackStreamRequest {
        using this_type = FeedbackStreamRequest<SubjectType_>;

        template <typename Type_>
        using wrapper_type = typename LoaderStreamRequest<SubjectType_>::template wrapper_type<Type_>;
        using value_type = typename LoaderStreamRequest<SubjectType_>::template value_type;

        using type = wrapper_type<value_type>;

        /**/

        template <typename Type_>
        using options_wrapper_type = _STD type_identity_t<Type_>;
        using options_type = typename StreamOptions<this_type>::type;

        using options = options_wrapper_type<options_type>;
    };

    template <IsStreamResponseSubjectType SubjectType_>
    struct FeedbackStreamResponse {
        using this_type = FeedbackStreamResponse<SubjectType_>;

        template <typename Type_>
        using wrapper_type = typename LoaderStreamResponse<SubjectType_>::template wrapper_type<Type_>;
        using value_type = typename LoaderStreamResponse<SubjectType_>::template value_type;

        using type = wrapper_type<value_type>;
    };

    /**/

    template <typename AssetType_, typename ResourceType_, bool Streamable_ = IsStreamRequestSubjectType<AssetType_>>
    struct __declspec(novtable) FeedbackNextLink {
        using this_type = FeedbackNextLink<AssetType_, ResourceType_, Streamable_>;

        using feedback_type = Feedback<AssetType_, ResourceType_>;
        using feedback_request = typename feedback_type::request_type;
        using feedback_response = typename feedback_type::response_type;

        using next_request_type = TransformerRequest<AssetType_>;
        using next_response_type = TransformerResponse<ResourceType_>;

        using next_stream_request_type = void;
        using next_stream_response_type = void;

        using loader_traits = LoaderTraits<AssetType_, ResourceType_>;

    public:
        virtual ~FeedbackNextLink() noexcept = default;

    public:
        [[nodiscard]] virtual typename next_response_type::type operator()(
            _In_ mref<typename next_request_type::type> request_,
            _In_ mref<typename next_request_type::options> options_
        ) const noexcept = 0;
    };

    template <typename AssetType_, typename ResourceType_>
    struct __declspec(novtable) FeedbackNextLink<AssetType_, ResourceType_, true> {
        using this_type = FeedbackNextLink<AssetType_, ResourceType_, true>;

        using feedback_type = Feedback<AssetType_, ResourceType_>;
        using feedback_request = typename feedback_type::request_type;
        using feedback_response = typename feedback_type::response_type;

        using next_request_type = TransformerRequest<AssetType_>;
        using next_response_type = TransformerResponse<ResourceType_>;

        using next_stream_request_type = TransformerStreamRequest<AssetType_>;
        using next_stream_response_type = TransformerStreamResponse<AssetType_>;

        using loader_traits = LoaderTraits<AssetType_, ResourceType_>;

    public:
        virtual ~FeedbackNextLink() noexcept = default;

    public:
        [[nodiscard]] virtual typename next_response_type::type operator()(
            _In_ mref<typename next_request_type::type> request_,
            _In_ mref<typename next_request_type::options> options_
        ) const noexcept = 0;

        [[nodiscard]] virtual typename next_stream_response_type::type operator()(
            _In_ mref<typename next_stream_request_type::type> request_,
            _In_ mref<typename next_stream_request_type::options> options_
        ) const noexcept = 0;
    };
}

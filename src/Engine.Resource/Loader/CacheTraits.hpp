#pragma once

#include "__fwd.hpp"
#include "LoaderTraits.hpp"

namespace hg::engine::resource::loader {
    template <typename AssetType_, typename ResourceType_>
    class Cache;

    /**/

    template <IsRequestValueType AssetType_>
    struct CacheRequest {
        using this_type = CacheRequest<AssetType_>;

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

    template <IsStreamRequestSubjectType SubjectType_>
    struct CacheStreamRequest {
        using this_type = CacheStreamRequest<SubjectType_>;

        template <typename Type_>
        using wrapper_type = typename LoaderStreamRequest<SubjectType_>::template wrapper_type<Type_>;
        using value_type = typename LoaderStreamRequest<SubjectType_>::value_type;

        using type = wrapper_type<value_type>;

        /**/

        template <typename Type_>
        using options_wrapper_type = _STD type_identity_t<Type_>;
        using options_type = typename StreamOptions<this_type>::type;

        using options = options_wrapper_type<options_type>;
    };

    template <IsResponseValueType ResourceType_>
    struct CacheResponse {
        using this_type = CacheResponse<ResourceType_>;

        template <typename Type_>
        using wrapper_type = typename LoaderResponse<ResourceType_>::template wrapper_type<Type_>;
        using value_type = ResourceType_;

        using type = wrapper_type<value_type>;
    };

    template <IsStreamResponseSubjectType SubjectType_>
    struct CacheStreamResponse {
        using this_type = CacheStreamResponse<SubjectType_>;

        template <typename Type_>
        using wrapper_type = typename LoaderStreamResponse<SubjectType_>::template wrapper_type<Type_>;
        using value_type = typename LoaderStreamResponse<SubjectType_>::value_type;

        using type = wrapper_type<value_type>;
    };

    /**/

    template <typename AssetType_, typename ResourceType_, bool Streamable_>
    struct __declspec(novtable) CacheNextLink {
        using this_type = CacheNextLink<AssetType_, ResourceType_, Streamable_>;

        using cache_type = Cache<AssetType_, ResourceType_>;
        using cache_request = typename cache_type::request_type;
        using cache_response = typename cache_type::response_type;

        using next_request_type = FeedbackRequest<AssetType_>;
        using next_response_type = FeedbackResponse<ResourceType_>;

        using next_stream_request_type = void;
        using next_stream_response_type = void;

        using loader_traits = LoaderTraits<AssetType_, ResourceType_>;

    public:
        virtual ~CacheNextLink() noexcept = default;

    public:
        [[nodiscard]] virtual typename next_response_type::type operator()(
            _In_ mref<typename next_request_type::type> request_,
            _In_ mref<typename next_request_type::options> options_
        ) const noexcept = 0;
    };

    template <typename AssetType_, typename ResourceType_>
    struct __declspec(novtable) CacheNextLink<AssetType_, ResourceType_, true> {
        using this_type = CacheNextLink<AssetType_, ResourceType_, true>;

        using cache_type = Cache<AssetType_, ResourceType_>;
        using cache_request = typename cache_type::request_type;
        using cache_response = typename cache_type::response_type;

        using next_request_type = FeedbackRequest<AssetType_>;
        using next_response_type = FeedbackResponse<ResourceType_>;

        using next_stream_request_type = FeedbackStreamRequest<AssetType_>;
        using next_stream_response_type = FeedbackStreamResponse<AssetType_>;

        using loader_traits = LoaderTraits<AssetType_, ResourceType_>;

    public:
        virtual ~CacheNextLink() noexcept = default;

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

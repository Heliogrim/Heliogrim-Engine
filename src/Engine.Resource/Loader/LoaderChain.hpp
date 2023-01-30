#pragma once

#include "__fwd.hpp"

#include "LoaderTraits.hpp"

#include "Loader.hpp"
#include "Cache.hpp"
#include "Feedback.hpp"
#include "SourceLoader.hpp"
#include "Transformer.hpp"

namespace ember::engine::resource::loader {
    template <
        template <typename> typename Type_,
        template <typename> typename... Rest_
    >
    struct MetaTypeWrapperList {
        using next = MetaTypeWrapperList<Rest_...>;

        template <typename Ty_>
        using type = Type_<Ty_>;

        constexpr size_t count() const noexcept {
            return next::count() + 1;
        }
    };

    template <template <typename> typename Type_>
    struct MetaTypeWrapperList<Type_> {
        using next = void;
        template <typename Ty_>
        using type = Type_<Ty_>;

        constexpr size_t count() const noexcept {
            return 1;
        }
    };

    template <
        IsRequestValueType RequestType_,
        IsResponseValueType ResponseType_ = ResourceBase,
        typename StageRefTypes_ = MetaTypeWrapperList<
            _STD type_identity_t,
            _STD type_identity_t,
            _STD type_identity_t,
            sptr
        >,
        class CacheStageType_ = Cache<RequestType_, ResponseType_>,
        class FeedbackStageType_ = Feedback<RequestType_, ResponseType_>,
        class TransformerStageType_ = Transformer<RequestType_, ResponseType_>,
        class SourceLoaderType_ = SourceLoader>
    class LoaderChain :
        public Loader<RequestType_, ResponseType_> {
    public:
        using this_type = LoaderChain<
            RequestType_,
            ResponseType_,
            StageRefTypes_,
            CacheStageType_,
            FeedbackStageType_,
            TransformerStageType_,
            SourceLoaderType_
        >;
        using underlying_type = Loader<RequestType_, ResponseType_>;

        using traits = LoaderTraits<RequestType_, ResponseType_>;

        /**/

        using cache_ref = typename StageRefTypes_;
        template <typename Type_>
        using cache_ref_type = typename cache_ref::template type<Type_>;
        using cache_stage_type = CacheStageType_;

        using cache_type = cache_ref_type<cache_stage_type>;

        using feedback_ref = typename cache_ref::next;
        template <typename Type_>
        using feedback_ref_type = typename feedback_ref::template type<Type_>;
        using feedback_stage_type = FeedbackStageType_;

        using feedback_type = feedback_ref_type<feedback_stage_type>;

        using transformer_ref = typename feedback_ref::next;
        template <typename Type_>
        using transformer_ref_type = typename transformer_ref::template type<Type_>;
        using transformer_stage_type = TransformerStageType_;

        using transformer_type = transformer_ref_type<transformer_stage_type>;

        using source_loader_ref = typename transformer_ref::next;
        template <typename Type_>
        using source_loader_ref_type = typename source_loader_ref::template type<Type_>;
        using source_loader_stage_type = SourceLoaderType_;

        using source_loader_type = source_loader_ref_type<source_loader_stage_type>;

    public:
        cache_type cache;
        feedback_type feedback;
        transformer_type transformer;
        source_loader_type sourceLoader;

    public:
        template <
            typename CacheType_,
            typename FeedbackType_,
            typename TransformerType_,
            typename SourceLoaderType_
        >
        constexpr LoaderChain(
            CacheType_&& cache_,
            FeedbackType_&& feedback_,
            TransformerType_&& transformer_,
            SourceLoaderType_&& sourceLoader_
        ) :
            cache(_STD forward<CacheType_>(cache_)),
            feedback(_STD forward<FeedbackType_>(feedback_)),
            transformer(_STD forward<TransformerType_>(transformer_)),
            sourceLoader(_STD forward<SourceLoaderType_>(sourceLoader_)),
            transformerLink(transformer),
            feedbackLink(feedback, transformerLink),
            cacheLink(cache, cacheLink) {}

    public:
        [[nodiscard]] typename traits::response::type operator()(
            mref<typename traits::request::type> request_,
            mref<typename traits::request::options> options_
        ) override {
            return invoke_stage<cache_stage_type, traits::response::type>(
                cache,
                _STD move(request_),
                _STD move(options_),
                cacheLink
            );
        }

        [[nodiscard]] typename traits::response::type operator()(
            mref<typename traits::request::type> request_,
            mref<typename traits::request::options> options_,
            mref<typename traits::request::stream> streamOptions_
        ) override;

    private:
        /* Stage Invoker Helper */

        template <typename Stage_, typename Result_, typename... Args_>
        [[nodiscard]] FORCE_INLINE static Result_ invoke_stage(cref<_STD decay_t<Stage_>> stage_, Args_&&... args_) {
            return stage_(_STD forward<Args_>(args_)...);
        }

        template <typename Stage_, typename Result_, typename... Args_>
        [[nodiscard]] FORCE_INLINE static Result_ invoke_stage(
            cref<uptr<_STD decay_t<Stage_>>> stage_,
            Args_&&... args_
        ) {
            return invoke_stage<Stage_, Result_, Args_...>(*stage_, _STD forward<Args_>(args_)...);
        }

        template <typename Stage_, typename Result_, typename... Args_>
        [[nodiscard]] FORCE_INLINE static Result_ invoke_stage(
            cref<sptr<_STD decay_t<Stage_>>> stage_,
            Args_&&... args_
        ) {
            return invoke_stage<Stage_, Result_, Args_...>(*stage_, _STD forward<Args_>(args_)...);
        }

        template <typename Stage_, typename Result_, typename... Args_>
        [[nodiscard]] FORCE_INLINE static Result_ invoke_stage(
            cref<smr<_STD decay_t<Stage_>>> stage_,
            Args_&&... args_
        ) {
            return invoke_stage<Stage_, Result_, Args_...>(*stage_, _STD forward<Args_>(args_)...);
        }

        /* Chain Link Implementations */

        struct TransformerChainLink :
            public TransformerNextLink<RequestType_, ResponseType_> {
            using base_type = TransformerNextLink<RequestType_, ResponseType_>;

            using base_type::next_response_type;
            using base_type::next_request_type;

        public:
            ~TransformerChainLink() override = default;

        public:
            cref<source_loader_type> stage;

        public:
            [[nodiscard]] typename next_response_type::type operator()(
                _In_ mref<typename next_request_type::type> request_,
                _In_ mref<typename next_request_type::options> options_
            ) const noexcept override {
                return invoke_stage<source_loader_stage_type, next_response_type::type>(
                    stage,
                    _STD move(request_),
                    _STD move(options_)
                );
            };

            [[nodiscard]] typename next_response_type::type operator()(
                _In_ mref<typename next_request_type::type> request_,
                _In_ mref<typename next_request_type::options> options_,
                _In_ mref<typename next_request_type::stream> streamOptions_
            ) const noexcept override {
                return invoke_stage<source_loader_stage_type, next_response_type::type>(
                    stage,
                    _STD move(request_),
                    _STD move(options_),
                    _STD move(streamOptions_)
                );
            };
        };

        struct FeedbackChainLink :
            public FeedbackNextLink<RequestType_, ResponseType_> {
            using base_type = FeedbackNextLink<RequestType_, ResponseType_>;

            using base_type::next_response_type;
            using base_type::next_request_type;

        public:
            ~FeedbackChainLink() override = default;

        public:
            cref<transformer_type> stage;
            cref<TransformerChainLink> nextLink;

        public:
            [[nodiscard]] typename next_response_type::type operator()(
                _In_ mref<typename next_request_type::type> request_,
                _In_ mref<typename next_request_type::options> options_
            ) const noexcept override {
                return invoke_stage<transformer_stage_type, next_response_type::type>(
                    stage,
                    _STD move(request_),
                    _STD move(options_),
                    nextLink
                );
            };

            [[nodiscard]] typename next_response_type::type operator()(
                _In_ mref<typename next_request_type::type> request_,
                _In_ mref<typename next_request_type::options> options_,
                _In_ mref<typename next_request_type::stream> streamOptions_
            ) const noexcept override {
                return invoke_stage<transformer_stage_type, next_response_type::type>(
                    stage,
                    _STD move(request_),
                    _STD move(options_),
                    _STD move(streamOptions_),
                    nextLink
                );
            };
        };

        struct CacheChainLink :
            public CacheNextLink<RequestType_, ResponseType_> {
            using base_type = CacheNextLink<RequestType_, ResponseType_>;

            using base_type::next_response_type;
            using base_type::next_request_type;

        public:
            ~CacheChainLink() override = default;

        public:
            cref<feedback_type> stage;
            cref<FeedbackChainLink> nextLink;

        public:
            [[nodiscard]] typename next_response_type::type operator()(
                _In_ mref<typename next_request_type::type> request_,
                _In_ mref<typename next_request_type::options> options_
            ) const noexcept override {
                return invoke_stage<feedback_stage_type, next_response_type::type>(
                    stage,
                    _STD move(request_),
                    _STD move(options_),
                    nextLink
                );
            };

            [[nodiscard]] typename next_response_type::type operator()(
                _In_ mref<typename next_request_type::type> request_,
                _In_ mref<typename next_request_type::options> options_,
                _In_ mref<typename next_request_type::stream> streamOptions_
            ) const noexcept override {
                return invoke_stage<feedback_stage_type, next_response_type::type>(
                    stage,
                    _STD move(request_),
                    _STD move(options_),
                    _STD move(streamOptions_),
                    nextLink
                );
            };
        };

    private:
        TransformerChainLink transformerLink;
        FeedbackChainLink feedbackLink;
        CacheChainLink cacheLink;
    };
}

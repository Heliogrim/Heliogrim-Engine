#pragma once

#include "__fwd.hpp"

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
        class FeedbackStageType_ = Feedback<
            typename CacheStageType_::sub_request_type::value_type,
            typename CacheStageType_::sub_response_type::value_type
        >,
        class TransformerStageType_ = Transformer<
            typename FeedbackStageType_::sub_request_type::value_type,
            typename FeedbackStageType_::sub_response_type::value_type
        >,
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

        using cache_ref = typename StageRefTypes_;
        template <typename Type_>
        using cache_ref_type = typename cache_ref::template type<Type_>;
        using cache_stage_type = CacheStageType_;

        using feedback_ref = typename cache_ref::next;
        template <typename Type_>
        using feedback_ref_type = typename feedback_ref::template type<Type_>;
        using feedback_stage_type = FeedbackStageType_;

        using transformer_ref = typename feedback_ref::next;
        template <typename Type_>
        using transformer_ref_type = typename transformer_ref::template type<Type_>;
        using transformer_stage_type = TransformerStageType_;

        using source_loader_ref = typename transformer_ref::next;
        template <typename Type_>
        using source_loader_ref_type = typename source_loader_ref::template type<Type_>;
        using source_loader_type = SourceLoaderType_;

        using request_value_type = RequestType_;
        using request_type = typename cache_stage_type::traits::request_type;

        using response_value_type = ResponseType_;
        using response_type = typename cache_stage_type::traits::response_type;

    public:
        cache_ref_type<cache_stage_type> cache;
        feedback_ref_type<feedback_stage_type> feedback;
        transformer_ref_type<transformer_stage_type> transformer;
        source_loader_ref_type<source_loader_type> sourceLoader;

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
            transformer(_STD forward<TransformerType_>(transformer)),
            sourceLoader(_STD forward<SourceLoaderType_>(sourceLoader_)) {}
    };
}

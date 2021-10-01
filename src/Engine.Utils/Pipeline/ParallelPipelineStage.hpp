#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/Future.hpp>

#include "AdaptivePipelineStage.hpp"

namespace ember::engine::utils::pipeline::adaptive {

    template <typename ResultType_, typename PushType_, typename PushCtxType_ = void, typename PullCtxType_ = void>
    class ParallelPipelineStage :
        public AdaptivePipelineStage<ResultType_, PushType_, PushCtxType_, PullCtxType_> {
    public:
        using base_type = AdaptivePipelineStage<ResultType_, PushType_, PushCtxType_, PullCtxType_>;

        using value_type = ParallelPipelineStage<ResultType_, PushType_, PushCtxType_, PullCtxType_>;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        using base_type::result_type;
        using base_type::push_value_type;
        using base_type::push_ctx_type;
        using base_type::pull_ctx_type;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 08.09.2021
         */
        ParallelPipelineStage() noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 08.09.2021
         */
        ~ParallelPipelineStage() noexcept = default;

    public:
        template <typename = _STD enable_if_t<!_STD is_void_v<PushCtxType_>>>
        [[nodiscard]] concurrent::future<result_type> push(ptr<push_value_type> ptr_, mref<push_ctx_type> ctx_) { }

        template <typename = _STD enable_if_t<_STD is_void_v<PushCtxType_>>>
        [[nodiscard]] concurrent::future<result_type> push(ptr<push_value_type> ptr_) = 0;

    public:
        template <typename = _STD enable_if_t<!_STD is_void_v<PullCtxType_>>>
        [[nodiscard]] concurrent::future<result_type> pull(mref<pull_ctx_type> ctx_) = 0;

        template <typename = _STD enable_if_t<_STD is_void_v<PullCtxType_>>>
        [[nodiscard]] concurrent::future<result_type> pull() = 0;
    };
}

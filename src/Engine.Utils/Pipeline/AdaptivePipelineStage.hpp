#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "AtomicPipelineStage.hpp"

namespace hg::engine::utils::pipeline::adaptive {
    template <typename ResultType_, typename PushType_, typename PushCtxType_, typename PullCtxType_>
    class AdaptivePipelineStage :
        protected AtomicPipelineStage {
    public:
        using base_type = AtomicPipelineStage;

        using value_type = AdaptivePipelineStage;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        using result_type = ResultType_;
        using push_value_type = PushType_;
        using push_ctx_type = PushCtxType_;
        using pull_ctx_type = PullCtxType_;

    protected:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 08.09.2021
         */
        AdaptivePipelineStage() noexcept = default;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 08.09.2021
         */
        ~AdaptivePipelineStage() noexcept = default;

    private:
    };
}

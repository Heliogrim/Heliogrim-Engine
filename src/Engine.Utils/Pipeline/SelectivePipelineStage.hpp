#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "AdaptivePipelineStage.hpp"

namespace hg::engine::utils::pipeline::adaptive {
    template <typename SrcType_, typename DstType_>
    class SelectivePipelineStage :
        public AdaptivePipelineStage<SrcType_, DstType_> {
    public:
        using base_type = AdaptivePipelineStage<SrcType_, DstType_>;

        using value_type = SelectivePipelineStage<SrcType_, DstType_>;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 08.09.2021
         */
        SelectivePipelineStage() noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 08.09.2021
         */
        ~SelectivePipelineStage() noexcept = default;
    };
}

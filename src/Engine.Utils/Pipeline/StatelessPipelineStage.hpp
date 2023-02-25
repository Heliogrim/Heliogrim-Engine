#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "AtomicPipelineStage.hpp"

namespace ember::engine::utils::pipeline {
    class StatelessPipelineStage :
        protected AtomicPipelineStage {
    public:
        using base_type = AtomicPipelineStage;

        using value_type = StatelessPipelineStage;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 08.09.2021
         */
        StatelessPipelineStage() noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 08.09.2021
         */
        ~StatelessPipelineStage() noexcept = default;
    };
}

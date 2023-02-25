#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::utils::pipeline {
    class StatelessPipeline {
    public:
        using value_type = StatelessPipeline;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 08.09.2021
         */
        StatelessPipeline() noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 08.09.2021
         */
        ~StatelessPipeline() noexcept = default;
    };
}

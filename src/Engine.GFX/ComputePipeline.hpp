#pragma once
#include "Shader/ShaderSlot.hpp"

namespace ember::engine::gfx {

    class ComputePipeline {
    protected:
        /**
         * Constructor
         *
         * @author Julius
         * @date 10.08.2022
         */
        ComputePipeline() = default;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 10.08.2022
         */
        virtual ~ComputePipeline() noexcept = default;

    public:
        /**
         * Setup
         *
         * @author Julius
         * @date 10.08.2022
         */
        virtual void setup() = 0;

        /**
         * Destroy
         *
         * @author Julius
         * @date 10.08.2022
         */
        virtual void destroy() noexcept = 0;

    protected:
        /**
         * Shader use for compute pipeline
         */
        ShaderSlot _shaderSlot = { shader::ShaderType::eCompute, "" };

    public:
        [[nodiscard]] cref<ShaderSlot> shaderSlot() const noexcept;

        [[nodiscard]] ref<ShaderSlot> shaderSlot() noexcept;
    };
}

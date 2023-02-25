#pragma once

#include "../Shader/ShaderSlot.hpp"

namespace ember::engine::gfx::pipeline {
    class VertexStage final {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 03.12.2020
         */
        VertexStage() noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 01.12.2020
         */
        ~VertexStage() noexcept = default;

    private:
        /**
         * Vertex Shader
         */
        ShaderSlot _shaderSlot = { shader::ShaderType::eVertex, "" };

    public:
        /**
         * Gets the shader
         *
         * @author Julius
         * @date 01.12.2020
         *
         * @returns A reference to a const Shader.
         */
        [[nodiscard]] const ShaderSlot& shaderSlot() const noexcept;

        /**
         * Shader slot
         *
         * @author Julius
         * @date 17.12.2020
         *
         * @returns A reference to a ShaderSlot.
         */
        [[nodiscard]] ShaderSlot& shaderSlot() noexcept;
    };
}

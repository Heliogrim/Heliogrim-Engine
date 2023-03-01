#pragma once

#include "../Shader/ShaderSlot.hpp"

namespace hg::engine::gfx::pipeline {
    class TessellationStage final {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 03.12.2020
         */
        TessellationStage() noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 01.12.2020
         */
        ~TessellationStage() noexcept = default;

    private:
        /**
         * Tessellation Shader
         */
        ShaderSlot _ctrlShader = { shader::ShaderType::eTessellation, "" };
        ShaderSlot _evalShader = { shader::ShaderType::eTessellation, "" };

    public:
        /**
         * Control shader
         *
         * @author Julius
         * @date 01.12.2020
         *
         * @returns A reference to a const Shader.
         */
        [[nodiscard]] const ShaderSlot& ctrlShaderSlot() const noexcept;

        /**
         * Evaluation shader
         *
         * @author Julius
         * @date 01.12.2020
         *
         * @returns A reference to a const Shader.
         */
        [[nodiscard]] const ShaderSlot& evalShaderSlot() const noexcept;
    };
}

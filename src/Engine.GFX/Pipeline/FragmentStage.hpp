#pragma once

#include "../Shader/ShaderSlot.hpp"

namespace hg::engine::gfx::pipeline {
    class FragmentStage final {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 03.12.2020
         */
        FragmentStage() noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 01.12.2020
         */
        ~FragmentStage() noexcept = default;

    private:
        /**
         * Fragment Shader
         */
        ShaderSlot _shaderSlot = { shader::ShaderType::eFragment, "" };

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
         * @date 03.12.2020
         *
         * @returns A reference to a ShaderSlot.
         */
        [[nodiscard]] ShaderSlot& shaderSlot() noexcept;
    };
}

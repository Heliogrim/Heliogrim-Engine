#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "ShaderBinding.hpp"
#include "Type.hpp"

namespace hg::engine::gfx::shader {
    /**
     * Forward Declaration
     */
    class Factory;
}

namespace hg::engine::gfx {
    class Shader {
    protected:
        friend hg::engine::gfx::shader::Factory;

        /**
         * Constructor
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @param  type_ The type.
         * @param  name_ The name.
         * @param  bindings_ The bindings.
         * @param  vkShader_ The shader.
         */
        Shader(
            const shader::ShaderType type_,
            const string& name_,
            const Vector<shader::ShaderBinding>& bindings_,
            vk::ShaderModule vkShader_
        );

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 03.12.2020
         */
        Shader() noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 25.11.2020
         */
        ~Shader() noexcept = default;

        /**
         * Cast that converts the given to a bool
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @returns The result of the operation.
         */
        operator bool() const noexcept;

    private:
        /**
         * Shader Type
         */
        shader::ShaderType _type;

    public:
        /**
         * Gets the type
         *
         * @author Julius
         * @date 25.11.2020
         *
         * @returns A shader::ShaderType.
         */
        [[nodiscard]] shader::ShaderType type() const noexcept;

    private:
        /**
         * Shader Name
         */
        string _name;

    public:
        /**
         * Gets the name
         *
         * @author Julius
         * @date 30.11.2020
         *
         * @returns A reference to a const string.
         */
        [[nodiscard]] const string& name() const noexcept;

    private:
        /**
         * Shader Bindings
         */
        Vector<shader::ShaderBinding> _bindings;

    public:
        /**
         * Gets the shader bindings
         *
         * @author Julius
         * @date 27.11.2020
         *
         * @returns A list of.
         */
        [[nodiscard]] const Vector<shader::ShaderBinding>& bindings() const noexcept;

    private:
        /**
         * Vulkan API
         */
        vk::ShaderModule _vkShader;

    public:
        /**
         * Gets the PipelineShaderStageCreateInfo
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @returns A vk::PipelineShaderStageCreateInfo.
         */
        [[nodiscard]] vk::PipelineShaderStageCreateInfo vkShaderStageInfo() const noexcept;
    };
}

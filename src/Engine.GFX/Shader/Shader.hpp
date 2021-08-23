#pragma once

#include <Engine.Common/Make.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/List.hpp>

#include "Binding.hpp"
#include "Type.hpp"
#include "../Buffer/Buffer.hpp"
#include "../Texture/Texture.hpp"

namespace ember::engine::gfx::shader {
    /**
     * Forward Declaration
     */
    class Factory;
}

namespace ember::engine::gfx {
    class Shader {
    protected:
        friend ember::engine::gfx::shader::Factory;

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
        Shader(const shader::ShaderType type_, const string& name_,
            const vector<shader::Binding>& bindings_,
            vk::ShaderModule vkShader_);

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

        /**
         * Binds
         *
         * @author Julius
         * @date 18.11.2020
         *
         * @param  id_ The identifier.
         * @param  buffer_ The buffer.
         */
        void bind(u32 id_, Buffer::const_reference_type buffer_);

        /**
         * Binds
         *
         * @author Julius
         * @date 17.11.2020
         *
         * @param  id_ The identifier.
         * @param  texture_ The texture.
         */
        void bind(u32 id_, Texture::const_reference_type texture_);

    private:
        /**
         * Bindings
         */
        vector<shader::Binding> _bindings;

    public:
        /**
         * Gets the bindings
         *
         * @author Julius
         * @date 27.11.2020
         *
         * @returns A list of.
         */
        [[nodiscard]] const vector<shader::Binding>& bindings() const noexcept;

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

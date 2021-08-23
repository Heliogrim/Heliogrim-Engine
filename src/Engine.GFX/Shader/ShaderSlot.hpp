#pragma once

#include "Shader.hpp"

namespace ember::engine::gfx {

    class ShaderSlot {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @param  type_ The type.
         * @param  name_ The name.
         */
        ShaderSlot(const shader::ShaderType type_, const string& name_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 03.12.2020
         */
        ~ShaderSlot() noexcept = default;

    private:
        /**
         * Cache
         */
        mutable ptr<const Shader> _cache;

    public:

        /**
         * Gets the shader
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @param  bypass_ (Optional) True to bypass.
         *
         * @returns A ptr&lt;const Shader&gt;
         */
        ptr<const Shader> shader(bool bypass_ = false) const noexcept;

        /**
         * Gets the shader
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @param  bypass_ (Optional) True to bypass.
         *
         * @returns A ptr&lt;const Shader&gt;
         */
        ptr<const Shader> shader(bool bypass_ = false) noexcept;

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
         * @date 03.12.2020
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
         * @date 03.12.2020
         *
         * @returns A reference to a const string.
         */
        [[nodiscard]] const string& name() const noexcept;

        /**
         * Gets the name
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @returns A reference to a string.
         */
        [[nodiscard]] string& name() noexcept;
    };
}

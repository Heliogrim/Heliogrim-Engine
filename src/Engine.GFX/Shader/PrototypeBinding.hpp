#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include "Type.hpp"

namespace ember::engine::gfx::shader {
    class PrototypeBinding {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 27.11.2020
         *
         * @param  type_ The type.
         * @param  id_ The identifier.
         * @param  interval_ The interval.
         * @param  name_ The name.
         */
        PrototypeBinding(
            const BindingType& type_,
            const u32 id_,
            const BindingUpdateInterval interval_,
            const string& name_
        ) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 25.11.2020
         */
        ~PrototypeBinding() noexcept;

    private:
        /**
         * Type
         */
        BindingType _type;

    public:
        /**
         * Gets the type
         *
         * @author Julius
         * @date 26.11.2020
         *
         * @returns A BindingType.
         */
        [[nodiscard]] BindingType type() const noexcept;

    private:
        /**
         * The identifier
         */
        u32 _id;

    public:
        /**
         * Gets the identifier
         *
         * @author Julius
         * @date 27.11.2020
         *
         * @returns An u32.
         */
        [[nodiscard]] u32 id() const noexcept;

    private:
        /**
         * Update Interval
         */
        BindingUpdateInterval _interval;

    public:
        /**
         * Gets the update interval
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @returns A BindingUpdateInterval.
         */
        [[nodiscard]] BindingUpdateInterval updateInterval() const noexcept;

    private:
        /**
         * Name
         */
        string _name;

    public:
        /**
         * Gets the name
         *
         * @author Julius
         * @date 27.11.2020
         *
         * @returns A reference to a const string.
         */
        [[nodiscard]] const string& name() const noexcept;

    private:
        /**
         * Shader Type
         */
        ShaderType _shaderType;

    public:
        /**
         * Gets the shader type
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @returns A ShaderType.
         */
        [[nodiscard]] ShaderType shaderType() const noexcept;

        /**
         * Gets a mutable reference to the Shader type
         *
         * @author Julius
         * @date 17.12.2020
         *
         * @returns A reference to a ShaderType.
         */
        [[nodiscard]] ShaderType& shaderType() noexcept;
    };
}

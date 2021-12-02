#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include "PrototypeBinding.hpp"

namespace ember::engine::gfx::shader {

    class Prototype {
    public:

        /**
         * Default constructor
         *
         * @author Julius
         * @date 25.11.2020
         */
        Prototype() noexcept;

        /**
         * Constructor
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @param  type_ The type.
         * @param  name_ The name.
         */
        Prototype(const ShaderType& type_, const string& name_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 25.11.2020
         */
        ~Prototype() noexcept;

    private:
        /**
         * Type
         */
        ShaderType _type;

    public:
        /**
         * Gets the type
         *
         * @author Julius
         * @date 27.11.2020
         *
         * @returns A ShaderType.
         */
        [[nodiscard]] ShaderType type() const noexcept;

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
         * Prototype Bindings
         */
        Vector<PrototypeBinding> _bindings;

    public:
        /**
         * Gets the bindings
         *
         * @author Julius
         * @date 25.11.2020
         *
         * @returns A list of.
         */
        [[nodiscard]] const Vector<PrototypeBinding>& bindings() const noexcept;

        /**
         * Adds a binding
         *
         * @author Julius
         * @date 25.11.2020
         *
         * @param  binding_ The binding.
         */
        void add(const PrototypeBinding& binding_);

    private:
        /**
         * Code
         */
        ptr<void> _code;
        u64 _codeSize;

    public:
        /**
         * Code size
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @returns An u64.
         */
        [[nodiscard]] u64 codeSize() const noexcept;

        /**
         * Gets the code
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @returns Null if it fails, else a pointer to a const void.
         */
        [[nodiscard]] ptr<void> code() const noexcept;

        /**
         * Stores a code
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @param  data_ If non-null, the data.
         * @param  size_ The size.
         */
        void storeCode(ptr<void> data_, u64 size_) noexcept;
    };
}

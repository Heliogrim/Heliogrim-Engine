#pragma once

#include "../Asset.hpp"

namespace clockwork::__internal::assets {
    class SfxMaterial {
    public:
        static core::type_id type_id;

    private:
        /** Identifier for the type */
        core::type_id _typeId;

    public:
        /**
         * Gets type identifier
         *
         * @author Julius
         * @date 28.12.2020
         *
         * @returns The type identifier.
         */
        [[nodiscard]] core::type_id get_typeId() const noexcept;

    private:
        /** Unique identifier */
        asset_guid _guid;

    public:
        /**
         * Gets the unique identifier
         *
         * @author Julius
         * @date 28.12.2020
         *
         * @returns The unique identifier.
         */
        [[nodiscard]] asset_guid get_guid() const noexcept;
    };
}

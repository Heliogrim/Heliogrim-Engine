#pragma once

#include <Engine.Common/Meta/TypeId.hpp>

namespace ember::engine::scene {

    struct SceneGraphTagBase {
        constexpr SceneGraphTagBase() = delete;

        constexpr SceneGraphTagBase(const type_id typeId_) :
            _typeId(typeId_) {}

        /**
         * Identifier for the type
         */
        const type_id _typeId;

        /**
         * Gets type identifier
         *
         * @author Julius
         * @date 27.08.2021
         *
         * @returns The type identifier.
         */
        [[nodiscard]] constexpr type_id get_typeId() const noexcept {
            return _typeId;
        }

        /**
         * Equality operator
         *
         * @author Julius
         * @date 27.08.2021
         *
         * @param  other_ The other.
         *
         * @returns True if the parameters are considered equivalent.
         */
        [[nodiscard]] bool operator==(cref<SceneGraphTagBase> other_) const noexcept {
            return _typeId == other_._typeId;
        }

        /**
         * Inequality operator
         *
         * @author Julius
         * @date 27.08.2021
         *
         * @param  other_ The other.
         *
         * @returns True if the parameters are not considered equivalent.
         */
        [[nodiscard]] bool operator!=(cref<SceneGraphTagBase> other_) const noexcept {
            return _typeId != other_._typeId;
        }
    };

    template <typename DerivedType_>
    struct SceneGraphTag :
        SceneGraphTagBase {

        /**
         * Scene graph tag
         *
         * @author Julius
         * @date 27.08.2021
         */
        constexpr SceneGraphTag() :
            SceneGraphTagBase(DerivedType_::type_id) {}
    };
}

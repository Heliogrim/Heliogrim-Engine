#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

namespace ember::engine::scene {

    struct SceneTag {

        constexpr SceneTag() = delete;

        constexpr SceneTag(cref<type_id> typeId_) :
            _typeId(typeId_) {}

        /**
         * Runtime identifier for the type
         */
        const type_id _typeId;

        [[nodiscard]] constexpr type_id typeId() const noexcept {
            return _typeId;
        }

        [[nodiscard]] bool operator==(cref<SceneTag> other_) const noexcept {
            return _typeId == other_._typeId;
        }

        [[nodiscard]] bool operator!=(cref<SceneTag> other_) const noexcept {
            return _typeId != other_._typeId;
        }
    };
}

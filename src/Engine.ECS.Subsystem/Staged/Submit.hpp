#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.ECS/ComponentTypeId.hpp>
#include <Engine.ECS/Traits.hpp>

namespace ember::engine::ecs::subsystem::staged {

    struct Submit {
        entity_guid guid;
        component_type_id typeId;
        non_owning_rptr<void> component;

    public:
        constexpr bool operator<(cref<Submit> other_) const noexcept {
            return component < other_.component;
        }

        constexpr bool operator>(cref<Submit> other_) const noexcept {
            return component > other_.component;
        }

        constexpr bool operator==(cref<Submit> other_) const noexcept {
            return component == other_.component;
        }

        constexpr bool operator!=(cref<Submit> other_) const noexcept {
            return component != other_.component;
        }

        constexpr bool operator<=(cref<Submit> other_) const noexcept {
            return component <= other_.component;
        }

        constexpr bool operator>=(cref<Submit> other_) const noexcept {
            return component >= other_.component;
        }
    };
}

#pragma once

#include <Engine.Event/Event.hpp>

namespace ember::engine::core {
    class World;
}

namespace ember::engine::core {

    class WorldRemoveEvent :
        public StatelessEvent {
    public:
        using this_type = WorldRemoveEvent;

    public:
        inline static constexpr event_type_id typeId { "WorldRemoveEvent"_typeId };

    public:
        WorldRemoveEvent(cref<sptr<::ember::engine::core::World>> world_);

        WorldRemoveEvent(cref<this_type> other_) noexcept = default;

        WorldRemoveEvent(mref<this_type> other_) noexcept = default;

        ~WorldRemoveEvent() = default;

    private:
        sptr<::ember::engine::core::World> _world;

    public:
        [[nodiscard]] const sptr<::ember::engine::core::World> getWorld() const noexcept;
    };

}

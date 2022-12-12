#pragma once

#include <Engine.Event/Event.hpp>

namespace ember::engine::core {
    class World;
}

namespace ember::engine::core {
    class WorldAddedEvent :
        public StatelessEvent {
    public:
        using this_type = WorldAddedEvent;

    public:
        inline static constexpr event_type_id typeId { "WorldAddedEvent"_typeId };

    public:
        WorldAddedEvent(cref<sptr<::ember::engine::core::World>> world_);

        WorldAddedEvent(cref<this_type> other_) noexcept = default;

        WorldAddedEvent(mref<this_type> other_) noexcept = default;

        ~WorldAddedEvent() = default;

    private:
        sptr<::ember::engine::core::World> _world;

    public:
        [[nodiscard]] cref<sptr<::ember::engine::core::World>> getWorld() const noexcept;
    };
}

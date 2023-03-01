#pragma once

#include <Engine.Event/Event.hpp>

namespace hg::engine::core {
    class World;
}

namespace hg::engine::core {
    class WorldRemoveEvent :
        public StatelessEvent {
    public:
        using this_type = WorldRemoveEvent;

    public:
        inline static constexpr event_type_id typeId { "WorldRemoveEvent"_typeId };

    public:
        WorldRemoveEvent(cref<sptr<::hg::engine::core::World>> world_);

        WorldRemoveEvent(cref<this_type> other_) noexcept = default;

        WorldRemoveEvent(mref<this_type> other_) noexcept = default;

        ~WorldRemoveEvent() = default;

    private:
        sptr<::hg::engine::core::World> _world;

    public:
        [[nodiscard]] const sptr<::hg::engine::core::World> getWorld() const noexcept;
    };
}

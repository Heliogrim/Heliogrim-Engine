#pragma once

#include <Engine.Event/Event.hpp>

namespace ember::engine::core {
    class Session;
    class World;
}

namespace ember::engine::core {
    class WorldChangeEvent :
        public StatelessEvent {
    public:
        using this_type = WorldChangeEvent;

    public:
        inline static constexpr event_type_id typeId { "WorldChangeEvent"_typeId };

    public:
        WorldChangeEvent(
            const non_owning_rptr<Session> session_,
            cref<sptr<::ember::engine::core::World>> prevWorld_,
            cref<sptr<::ember::engine::core::World>> nextWorld_
        );

        WorldChangeEvent(cref<this_type> other_) noexcept = default;

        WorldChangeEvent(mref<this_type> other_) noexcept = default;

        ~WorldChangeEvent() = default;

    private:
        non_owning_rptr<Session> _session;
        sptr<World> _prevWorld;
        sptr<World> _nextWorld;

    public:
        [[nodiscard]] const non_owning_rptr<Session> getSession() const noexcept;

        [[nodiscard]] cref<sptr<World>> getPrevWorld() const noexcept;

        [[nodiscard]] cref<sptr<World>> getNextWorld() const noexcept;
    };
}

#pragma once

#include <Engine.Event/Event.hpp>

namespace ember::engine::scene {
    class Scene;
}

namespace ember::engine::core {

    class WorldAddedEvent :
        public StatelessEvent {
    public:
        using this_type = WorldAddedEvent;

    public:
        inline static constexpr event_type_id typeId { "WorldAddedEvent"_typeId };

    public:
        WorldAddedEvent(cref<sptr<::ember::engine::scene::Scene>> worldScene_);

        WorldAddedEvent(cref<this_type> other_) noexcept = default;

        WorldAddedEvent(mref<this_type> other_) noexcept = default;

        ~WorldAddedEvent() = default;

    private:
        sptr<::ember::engine::scene::Scene> _worldScene;

    public:
        [[nodiscard]] cref<sptr<::ember::engine::scene::Scene>> getWorldScene() const noexcept;
    };

}

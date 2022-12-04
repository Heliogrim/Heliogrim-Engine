#pragma once

#include <Engine.Event/Event.hpp>

namespace ember::engine::scene {
    class Scene;
}

namespace ember::engine::core {

    class WorldRemoveEvent :
        public StatelessEvent {
    public:
        using this_type = WorldRemoveEvent;

    public:
        inline static constexpr event_type_id typeId { "WorldRemoveEvent"_typeId };

    public:
        WorldRemoveEvent(const ptr<::ember::engine::scene::Scene> worldScene_);

        WorldRemoveEvent(cref<this_type> other_) noexcept = default;

        WorldRemoveEvent(mref<this_type> other_) noexcept = default;

        ~WorldRemoveEvent() = default;

    private:
        ptr<::ember::engine::scene::Scene> _worldScene;

    public:
        [[nodiscard]] const ptr<::ember::engine::scene::Scene> getWorldScene() const noexcept;
    };

}

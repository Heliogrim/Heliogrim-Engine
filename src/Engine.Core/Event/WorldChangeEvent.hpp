#pragma once

#include <Engine.Event/Event.hpp>

namespace ember::engine::scene {
    class Scene;
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
            const ptr<::ember::engine::scene::Scene> prevScene_,
            cref<sptr<::ember::engine::scene::Scene>> nextScene_
        );

        WorldChangeEvent(cref<this_type> other_) noexcept = default;

        WorldChangeEvent(mref<this_type> other_) noexcept = default;

        ~WorldChangeEvent() = default;

    private:
        ptr<::ember::engine::scene::Scene> _prevScene;
        sptr<::ember::engine::scene::Scene> _nextScene;

    public:
        [[nodiscard]] const ptr<::ember::engine::scene::Scene> getPrevScene() const noexcept;

        [[nodiscard]] cref<sptr<::ember::engine::scene::Scene>> getNextScene() const noexcept;
    };

}

#pragma once

#include <Engine.Event/Event.hpp>

namespace ember::engine::core {
    class EngineInitEvent final :
        public StatelessEvent {
    public:
        using this_type = EngineInitEvent;

    public:
        inline static constexpr event_type_id typeId { "EngineInitEvent"_typeId };

    public:
        EngineInitEvent() = default;

        ~EngineInitEvent() = default;
    };
}

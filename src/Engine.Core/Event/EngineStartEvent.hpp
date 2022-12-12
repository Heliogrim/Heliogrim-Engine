#pragma once

#include <Engine.Event/Event.hpp>

namespace ember::engine::core {
    class EngineStartEvent final :
        public StatelessEvent {
    public:
        using this_type = EngineStartEvent;

    public:
        inline static constexpr event_type_id typeId { "EngineStartEvent"_typeId };

    public:
        EngineStartEvent() = default;

        ~EngineStartEvent() = default;
    };
}

#pragma once

#include <Engine.Event/Event.hpp>

namespace ember::engine::core {
    class EngineStopEvent final :
        public StatelessEvent {
    public:
        using this_type = EngineStopEvent;

    public:
        inline static constexpr event_type_id typeId { "EngineStopEvent"_typeId };

    public:
        EngineStopEvent() = default;

        ~EngineStopEvent() = default;
    };
}

#pragma once

#include <Engine.Event/Event.hpp>

namespace ember::engine::core {
    class EnginePreInitEvent final :
        public StatelessEvent {
    public:
        using this_type = EnginePreInitEvent;

    public:
        inline static constexpr event_type_id typeId { "EnginePreInitEvent"_typeId };

    public:
        EnginePreInitEvent() = default;

        ~EnginePreInitEvent() = default;
    };
}

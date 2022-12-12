#pragma once

#include <Engine.Event/Event.hpp>

namespace ember::engine::core {
    class EnginePostInitEvent final :
        public StatelessEvent {
    public:
        using this_type = EnginePostInitEvent;

    public:
        inline static constexpr event_type_id typeId { "EnginePostInitEvent"_typeId };

    public:
        EnginePostInitEvent() = default;

        ~EnginePostInitEvent() = default;
    };
}

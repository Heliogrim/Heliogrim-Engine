#pragma once

#include <Engine.Event/Event.hpp>

namespace hg::engine::core {
    class EngineShutdownEvent final :
        public StatelessEvent {
    public:
        using this_type = EngineShutdownEvent;

    public:
        inline static constexpr event_type_id typeId { "EngineShutdownEvent"_typeId };

    public:
        EngineShutdownEvent() = default;

        ~EngineShutdownEvent() = default;
    };
}

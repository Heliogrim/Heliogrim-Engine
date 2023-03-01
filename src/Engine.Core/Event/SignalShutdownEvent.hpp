#pragma once

#include <Engine.Event/Event.hpp>

namespace hg::engine::core {
    /**
     * While the shutdown might be miss leading in regard of the engine lifecycle,
     *  this event is mostly used to signal system/engine wide shutdown process.
     *  In most cases the main thread will wait for this event to start the destructive
     *  shutdown in respect to the engine lifecycle.
     */
    class SignalShutdownEvent final :
        public StatelessEvent {
    public:
        using this_type = SignalShutdownEvent;

    public:
        inline static constexpr event_type_id typeId { "SignalShutdownEvent"_typeId };

    public:
        SignalShutdownEvent();

        ~SignalShutdownEvent() noexcept;
    };
}

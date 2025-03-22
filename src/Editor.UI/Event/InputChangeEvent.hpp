#pragma once

#include <Engine.Event/Event.hpp>

namespace hg::engine::reflow {
    class Widget;
}

namespace hg::engine::reflow {
    class InputChangeEvent :
        public StatelessEvent {
    public:
        using this_type = InputChangeEvent;

    public:
        inline static constexpr event_type_id typeId { "InputChangeEvent"_typeId };

    public:
        explicit InputChangeEvent(ref<const SharedPtr<Widget>> widget_) noexcept;

        ~InputChangeEvent() noexcept override = default;

    public:
        SharedPtr<Widget> target;
    };
}

#pragma once

#include <Engine.Event/Event.hpp>

namespace hg::engine::reflow {
    class Widget;
}

namespace hg::engine::reflow {
    class FocusEvent :
        public StatelessEvent {
    public:
        using this_type = FocusEvent;
        using reference_type = ref<this_type>;
        using const_reference_type = cref<this_type>;

    public:
        inline static constexpr event_type_id typeId { "FocusEvent"_typeId };

    public:
        explicit FocusEvent(cref<sptr<Widget>> widget_) noexcept;

        ~FocusEvent() noexcept override = default;

    private:
    public:
        sptr<Widget> _target;
    };
}

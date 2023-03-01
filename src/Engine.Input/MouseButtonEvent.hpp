#pragma once

#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Event/Event.hpp>

namespace hg::engine::input::event {
    class MouseButtonEvent final :
        public StatelessEvent {
    public:
        using this_type = MouseButtonEvent;
        using reference_type = ref<this_type>;
        using const_reference_type = cref<this_type>;

    public:
        inline static constexpr event_type_id typeId { "MouseButtonEvent"_typeId };

    public:
        explicit MouseButtonEvent(
            cref<math::ivec2> pointer_,
            const u32 button_,
            const bool down_,
            const u32 modifier_
        ) noexcept;

        MouseButtonEvent(cref<this_type> other_) noexcept = default;

        MouseButtonEvent(mref<this_type> other_) noexcept = default;

        ~MouseButtonEvent() noexcept = default;

        //private:
    public:
        math::ivec2 _pointer;
        u32 _button;
        bool _down;
        u32 _modifier;
    };
}

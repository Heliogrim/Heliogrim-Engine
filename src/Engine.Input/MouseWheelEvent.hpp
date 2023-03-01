#pragma once

#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Event/Event.hpp>

namespace hg::engine::input::event {
    class MouseWheelEvent final :
        public StatelessEvent {
    public:
        using this_type = MouseWheelEvent;
        using reference_type = ref<this_type>;
        using const_reference_type = cref<this_type>;

    public:
        inline static constexpr event_type_id typeId { "MouseWheelEvent"_typeId };

    public:
        explicit MouseWheelEvent(
            cref<math::ivec2> pointer_,
            cref<math::vec2> value_
        ) noexcept;

        MouseWheelEvent(cref<this_type> other_) noexcept = default;

        MouseWheelEvent(mref<this_type> other_) noexcept = default;

        ~MouseWheelEvent() noexcept = default;

        //private:
    public:
        math::ivec2 _pointer;
        math::vec2 _value;
    };
}

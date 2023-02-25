#pragma once

#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Event/Event.hpp>

namespace ember::engine::input::event {
    class KeyboardEvent final :
        public StatelessEvent {
    public:
        using this_type = KeyboardEvent;
        using reference_type = ref<this_type>;
        using const_reference_type = cref<this_type>;

    public:
        inline static constexpr event_type_id typeId { "KeyboardEvent"_typeId };

    public:
        explicit KeyboardEvent(
            const char key_,
            const bool down_,
            const u32 modifier_
        ) noexcept;

        KeyboardEvent(cref<this_type> other_) noexcept = default;

        KeyboardEvent(mref<this_type> other_) noexcept = default;

        ~KeyboardEvent() noexcept = default;

        //private:
    public:
        char _key;
        bool _down;
        u32 _modifier;
    };
}

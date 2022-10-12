#include "MouseButtonEvent.hpp"

using namespace ember::engine::input::event;
using namespace ember;

MouseButtonEvent::MouseButtonEvent(
    cref<math::ivec2> pointer_,
    const u32 button_,
    const bool down_,
    const u32 modifier_
) noexcept :
    _pointer(pointer_),
    _button(button_),
    _down(down_),
    _modifier(modifier_) {}

#include "MouseMoveEvent.hpp"

using namespace hg::engine::input::event;
using namespace hg;

MouseMoveEvent::MouseMoveEvent(
    cref<math::ivec2> pointer_,
    cref<math::ivec2> delta_,
    u32 button_,
    u32 modifier_
) noexcept :
    _pointer(pointer_),
    _delta(delta_),
    _button(button_),
    _modifier(modifier_) {}

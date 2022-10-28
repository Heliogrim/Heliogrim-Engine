#include "MouseWheelEvent.hpp"

using namespace ember::engine::input::event;
using namespace ember;

MouseWheelEvent::MouseWheelEvent(cref<math::ivec2> pointer_, cref<math::vec2> value_) noexcept :
    StatelessEvent(),
    _pointer(pointer_),
    _value(value_) {}

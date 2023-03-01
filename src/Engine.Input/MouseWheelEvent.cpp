#include "MouseWheelEvent.hpp"

using namespace hg::engine::input::event;
using namespace hg;

MouseWheelEvent::MouseWheelEvent(cref<math::ivec2> pointer_, cref<math::vec2> value_) noexcept :
    StatelessEvent(),
    _pointer(pointer_),
    _value(value_) {}

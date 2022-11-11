#include "KeyboardEvent.hpp"

using namespace ember::engine::input::event;
using namespace ember;

KeyboardEvent::KeyboardEvent(const char key_, const bool down_, const u32 modifier_) noexcept :
    StatelessEvent(),
    _key(key_),
    _down(down_),
    _modifier(modifier_) {}

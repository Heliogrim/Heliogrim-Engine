#include "KeyboardEvent.hpp"

using namespace hg::engine::input::event;
using namespace hg;

KeyboardEvent::KeyboardEvent(const char key_, const bool down_, const u32 modifier_) noexcept :
    StatelessEvent(),
    _key(key_),
    _down(down_),
    _modifier(modifier_) {}

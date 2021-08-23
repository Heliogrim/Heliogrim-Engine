#include "Registry.hpp"
#include <Engine.Common/Make.hpp>

using namespace ember::engine::ecs;

registry::this_type registry::_this = nullptr;

void registry::destroy() noexcept {
    _this.release();
}

registry& registry::get() noexcept {
    return *_this;
}

registry& registry::make() noexcept {
    if (!_this) {
        _this = _STD move(make_uptr<registry>());
    }
    return *_this;
}

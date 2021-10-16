#include "Registry.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>

using namespace ember::engine::ecs;
using namespace ember;

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

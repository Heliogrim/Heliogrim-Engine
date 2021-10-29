#include "Registry.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>

using namespace ember::engine::ecs;
using namespace ember;

registry::this_type registry::_this = nullptr;

registry::~registry() {

    /**
     * Cleanup adaptors
     */
    for (auto& entry : _adapters) {
        delete entry.second;
        entry.second = nullptr;
    }
}

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

pool_adapter* registry::get_pool_adapter(cref<component_type_id> typeId_) {
    const auto entry = _adapters.find(typeId_);

    if (entry != _adapters.end()) {
        return entry->second;
    }

    return nullptr;
}

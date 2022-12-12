#include "WorldAddedEvent.hpp"

using namespace ember::engine::core;
using namespace ember;

WorldAddedEvent::WorldAddedEvent(cref<sptr<::ember::engine::core::World>> world_) :
    StatelessEvent(),
    _world(world_) {}

cref<sptr<::ember::engine::core::World>> WorldAddedEvent::getWorld() const noexcept {
    return _world;
}

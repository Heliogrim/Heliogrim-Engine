#include "WorldRemoveEvent.hpp"

using namespace ember::engine::core;
using namespace ember;

WorldRemoveEvent::WorldRemoveEvent(cref<sptr<ember::engine::core::World>> world_) :
    StatelessEvent(),
    _world(world_) {}

const sptr<ember::engine::core::World> WorldRemoveEvent::getWorld() const noexcept {
    return _world;
}

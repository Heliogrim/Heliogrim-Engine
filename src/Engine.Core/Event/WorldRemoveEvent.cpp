#include "WorldRemoveEvent.hpp"

using namespace ember::engine::core;
using namespace ember;

WorldRemoveEvent::WorldRemoveEvent(const ptr<ember::engine::scene::Scene> worldScene_) :
    StatelessEvent(),
    _worldScene(worldScene_) {}

const ptr<ember::engine::scene::Scene> WorldRemoveEvent::getWorldScene() const noexcept {
    return _worldScene;
}

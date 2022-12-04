#include "WorldAddedEvent.hpp"

using namespace ember::engine::core;
using namespace ember;

WorldAddedEvent::WorldAddedEvent(cref<sptr<::ember::engine::scene::Scene>> worldScene_) :
    StatelessEvent(),
    _worldScene(worldScene_) {}

cref<sptr<::ember::engine::scene::Scene>> WorldAddedEvent::getWorldScene() const noexcept {
    return _worldScene;
}

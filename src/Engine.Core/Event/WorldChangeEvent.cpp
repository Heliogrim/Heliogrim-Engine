#include "WorldChangeEvent.hpp"

using namespace ember::engine::core;
using namespace ember;

WorldChangeEvent::WorldChangeEvent(
    const ptr<ember::engine::scene::Scene> prevScene_,
    cref<sptr<ember::engine::scene::Scene>> nextScene_
) :
    StatelessEvent(),
    _prevScene(prevScene_),
    _nextScene(nextScene_) {}

const ptr<ember::engine::scene::Scene> WorldChangeEvent::getPrevScene() const noexcept {
    return _prevScene;
}

cref<sptr<ember::engine::scene::Scene>> WorldChangeEvent::getNextScene() const noexcept {
    return _nextScene;
}

#include "WorldContext.hpp"

#include <Engine.Event/GlobalEventEmitter.hpp>

#include "Event/WorldChangeEvent.hpp"
#include "Engine.hpp"
#include "World.hpp"

using namespace hg::engine::core;
using namespace hg;

WorldContext::WorldContext(const non_owning_rptr<Session> owner_) :
    _owner(owner_),
    _world(nullptr),
    _worldType(WorldType::eUndefined) {}

WorldContext::~WorldContext() = default;

const non_owning_rptr<Session> WorldContext::getOwner() const noexcept {
    return _owner;
}

cref<sptr<World>> WorldContext::getCurrentWorld() const noexcept {
    return _world;
}

void WorldContext::setCurrentWorld(cref<sptr<World>> nextWorld_) {
    // Capture session, current world and next world to get stable references
    const WorldChangeEvent event { _owner, _world, nextWorld_ };

    /**/

    _world = nextWorld_;
    _worldType = (not nextWorld_) ? WorldType::eUndefined : WorldType::eGame;

    /**/

    Engine::getEngine()->getEmitter().emit(event);
}

WorldType WorldContext::getWorldType() const noexcept {
    return _worldType;
}

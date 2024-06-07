#include "WorldContext.hpp"

#include <Engine.Event/GlobalEventEmitter.hpp>

#include "Event/WorldChangeEvent.hpp"
#include "Engine.hpp"
#include "World.hpp"

using namespace hg::engine::core;
using namespace hg;

WorldContext::WorldContext(ref<Session> owner_) :
	_owner(std::addressof(owner_)),
	_world(nullptr),
	_worldType(WorldType::eUndefined) {}

WorldContext::~WorldContext() = default;

ref<Session> WorldContext::getOwner() const noexcept {
	return *_owner.get();
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

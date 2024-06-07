#include "WorldAddedEvent.hpp"

using namespace hg::engine::core;
using namespace hg;

WorldAddedEvent::WorldAddedEvent(cref<sptr<::hg::engine::core::World>> world_) :
	StatelessEvent(),
	_world(world_) {}

sptr<hg::engine::core::World> WorldAddedEvent::getWorld() const noexcept {
	return _world;
}

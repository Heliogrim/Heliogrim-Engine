#include "WorldRemoveEvent.hpp"

using namespace hg::engine::core;
using namespace hg;

WorldRemoveEvent::WorldRemoveEvent(cref<sptr<hg::engine::core::World>> world_) :
	StatelessEvent(),
	_world(world_) {}

sptr<hg::engine::core::World> WorldRemoveEvent::getWorld() const noexcept {
	return _world;
}

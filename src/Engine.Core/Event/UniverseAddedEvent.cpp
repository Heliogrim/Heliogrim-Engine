#include "UniverseAddedEvent.hpp"

using namespace hg::engine::core;
using namespace hg;

UniverseAddedEvent::UniverseAddedEvent(cref<sptr<::hg::engine::core::Universe>> universe_) :
	StatelessEvent(),
	_universe(universe_) {}

sptr<hg::engine::core::Universe> UniverseAddedEvent::getUniverse() const noexcept {
	return _universe;
}

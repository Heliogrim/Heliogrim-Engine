#include "UniverseRemoveEvent.hpp"

using namespace hg::engine::core;
using namespace hg;

UniverseRemoveEvent::UniverseRemoveEvent(cref<sptr<hg::engine::core::Universe>> universe_) :
	StatelessEvent(),
	_universe(universe_) {}

sptr<hg::engine::core::Universe> UniverseRemoveEvent::getUniverse() const noexcept {
	return _universe;
}

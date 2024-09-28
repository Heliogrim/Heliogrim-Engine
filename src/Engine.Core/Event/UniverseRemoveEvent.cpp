#include "UniverseRemoveEvent.hpp"

using namespace hg::engine::core;
using namespace hg;

UniverseRemoveEvent::UniverseRemoveEvent(mref<SharedPtr<hg::engine::core::Universe>> universe_) :
	StatelessEvent(),
	_universe(std::move(universe_)) {}

SharedPtr<hg::engine::core::Universe> UniverseRemoveEvent::getUniverse() const noexcept {
	return _universe;
}

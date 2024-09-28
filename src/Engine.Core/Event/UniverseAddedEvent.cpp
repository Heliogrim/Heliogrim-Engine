#include "UniverseAddedEvent.hpp"

using namespace hg::engine::core;
using namespace hg;

UniverseAddedEvent::UniverseAddedEvent(mref<SharedPtr<::hg::engine::core::Universe>> universe_) :
	StatelessEvent(),
	_universe(std::move(universe_)) {}

SharedPtr<hg::engine::core::Universe> UniverseAddedEvent::getUniverse() const noexcept {
	return _universe;
}

#include "UniverseContext.hpp"

#include <Engine.Event/GlobalEventEmitter.hpp>

#include "Engine.hpp"
#include "Universe.hpp"
#include "Event/UniverseChangeEvent.hpp"

using namespace hg::engine::core;
using namespace hg;

UniverseContext::UniverseContext(ref<Session> owner_) :
	_owner(std::addressof(owner_)),
	_universe(nullptr),
	_universeType(UniverseType::eUndefined) {}

UniverseContext::~UniverseContext() = default;

ref<Session> UniverseContext::getOwner() const noexcept {
	return *_owner.get();
}

cref<sptr<Universe>> UniverseContext::getCurrentUniverse() const noexcept {
	return _universe;
}

void UniverseContext::setCurrentUniverse(cref<sptr<Universe>> nextUniverse_) {
	// Capture session, current universe and next universe to get stable references
	const UniverseChangeEvent event { _owner, _universe, nextUniverse_ };

	/**/

	_universe = nextUniverse_;
	_universeType = (not nextUniverse_) ? UniverseType::eUndefined : UniverseType::eGame;

	/**/

	Engine::getEngine()->getEmitter().emit(event);
}

UniverseType UniverseContext::getUniverseType() const noexcept {
	return _universeType;
}

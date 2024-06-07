#include "WorldChangeEvent.hpp"

using namespace hg::engine::core;
using namespace hg;

WorldChangeEvent::WorldChangeEvent(
	nmpt<Session> session_,
	cref<sptr<World>> prevWorld_,
	cref<sptr<World>> nextWorld_
) :
	StatelessEvent(),
	_session(session_),
	_prevWorld(prevWorld_),
	_nextWorld(nextWorld_) {}

nmpt<Session> WorldChangeEvent::getSession() const noexcept {
	return _session;
}

sptr<World> WorldChangeEvent::getPrevWorld() const noexcept {
	return _prevWorld;
}

sptr<World> WorldChangeEvent::getNextWorld() const noexcept {
	return _nextWorld;
}

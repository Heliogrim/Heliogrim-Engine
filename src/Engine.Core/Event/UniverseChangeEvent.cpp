#include "UniverseChangeEvent.hpp"

using namespace hg::engine::core;
using namespace hg;

UniverseChangeEvent::UniverseChangeEvent(
	nmpt<Session> session_,
	cref<sptr<Universe>> prevUniverse_,
	cref<sptr<Universe>> nextUniverse_
) :
	StatelessEvent(),
	_session(session_),
	_prevUniverse(prevUniverse_),
	_nextUniverse(nextUniverse_) {}

nmpt<Session> UniverseChangeEvent::getSession() const noexcept {
	return _session;
}

sptr<Universe> UniverseChangeEvent::getPrevUniverse() const noexcept {
	return _prevUniverse;
}

sptr<Universe> UniverseChangeEvent::getNextUniverse() const noexcept {
	return _nextUniverse;
}

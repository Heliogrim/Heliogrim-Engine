#include "UniverseChangeEvent.hpp"

using namespace hg::engine::core;
using namespace hg;

UniverseChangeEvent::UniverseChangeEvent(
	nmpt<Session> session_,
	mref<SharedPtr<Universe>> prevUniverse_,
	mref<SharedPtr<Universe>> nextUniverse_
) :
	StatelessEvent(),
	_session(session_),
	_prevUniverse(std::move(prevUniverse_)),
	_nextUniverse(std::move(nextUniverse_)) {}

nmpt<Session> UniverseChangeEvent::getSession() const noexcept {
	return _session;
}

SharedPtr<Universe> UniverseChangeEvent::getPrevUniverse() const noexcept {
	return _prevUniverse;
}

SharedPtr<Universe> UniverseChangeEvent::getNextUniverse() const noexcept {
	return _nextUniverse;
}

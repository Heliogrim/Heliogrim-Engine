#include "Session.hpp"

#include <Engine.Common/__macro.hpp>

#include "SessionState.hpp"
#include "UniverseContext.hpp"

import Engine.Common;

using namespace hg::engine::core;
using namespace hg;

Session::Session() :
	_state(uptr<SessionState>(new SessionState())),
	_universeContext(make_uptr<UniverseContext>(*this)) {}

Session::~Session() = default;

ref<class SessionState> Session::getState() const noexcept {
	return *_state;
}

ref<class UniverseContext> Session::getUniverseContext() const noexcept {
	return *_universeContext;
}

#include "Session.hpp"

#include <Engine.Common/Make.hpp>

#include "SessionState.hpp"
#include "WorldContext.hpp"

using namespace hg::engine::core;
using namespace hg;

Session::Session() :
	_state(uptr<SessionState>(new SessionState())),
	_worldContext(make_uptr<WorldContext>(*this)) {}

Session::~Session() = default;

ref<class SessionState> Session::getState() const noexcept {
	return *_state;
}

ref<class WorldContext> Session::getWorldContext() const noexcept {
	return *_worldContext;
}

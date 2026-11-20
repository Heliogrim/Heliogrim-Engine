import Engine.Common;
import Engine.Core;

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

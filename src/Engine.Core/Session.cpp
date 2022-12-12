#include "Session.hpp"

#include <Engine.Common/Make.hpp>

#include "SessionState.hpp"
#include "WorldContext.hpp"

using namespace ember::engine::core;
using namespace ember;

Session::Session() :
    _state(uptr<SessionState>(new SessionState())),
    _worldContext(make_uptr<WorldContext>(this)) {}

Session::~Session() = default;

const non_owning_rptr<SessionState> Session::getState() const noexcept {
    return _state.get();
}

const non_owning_rptr<WorldContext> Session::getWorldContext() const noexcept {
    return _worldContext.get();
}

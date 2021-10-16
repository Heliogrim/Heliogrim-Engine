#include "Physics.hpp"

#include <Engine.Common/__macro.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include <Engine.Session/Session.hpp>

using namespace ember::engine;
using namespace ember;

Physics::Physics(cref<sptr<Session>> session_) noexcept :
    _session(session_) {}

Physics::~Physics() = default;

void Physics::setup() {

    SCOPED_STOPWATCH
}

void Physics::schedule() { }

sptr<Session> Physics::session() const noexcept {
    return _session;
}

#include "Network.hpp"

#include <Engine.Common/__macro.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace ember::engine;
using namespace ember;

Network::Network(cref<sptr<Session>> session_) noexcept :
    _session(session_) {}

Network::~Network() = default;

void Network::setup() {

    SCOPED_STOPWATCH
}

void Network::schedule() {}

sptr<Session> Network::session() const noexcept {
    return _session;
}

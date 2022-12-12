#include "Network.hpp"

#include <Engine.Common/__macro.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace ember::engine;
using namespace ember;

Network::Network(const non_owning_rptr<Engine> engine_) noexcept :
    CoreModule(engine_) {}

Network::~Network() = default;

void Network::setup() {
    SCOPED_STOPWATCH
}

void Network::schedule() {}

void Network::desync() {}

void Network::destroy() {}

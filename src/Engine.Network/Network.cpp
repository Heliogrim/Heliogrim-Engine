#include <Engine.Common/__macro.hpp>

#ifdef _PROFILING
#endif

import Engine.Common;
import Engine.Network;

using namespace hg::engine;
using namespace hg;

Network::Network(ref<Engine> engine_) noexcept :
	CoreModule(engine_) {}

Network::~Network() = default;

void Network::setup() {
	SCOPED_STOPWATCH
}

void Network::start() {}

void Network::stop() {}

void Network::destroy() {}

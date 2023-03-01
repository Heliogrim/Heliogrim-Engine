#include "Physics.hpp"

#include <Engine.Common/__macro.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace hg::engine;
using namespace hg;

Physics::Physics(const non_owning_rptr<Engine> engine_) noexcept :
    CoreModule(engine_) {}

Physics::~Physics() = default;

void Physics::setup() {
    SCOPED_STOPWATCH
}

void Physics::start() { }

void Physics::stop() {}

void Physics::destroy() {}

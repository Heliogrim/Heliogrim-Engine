#include "Physics.hpp"

#include <Engine.Common/__macro.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace ember::engine;
using namespace ember;

Physics::Physics(const non_owning_rptr<Engine> engine_) noexcept :
    CoreModule(engine_) {}

Physics::~Physics() = default;

void Physics::setup() {
    SCOPED_STOPWATCH
}

void Physics::schedule() { }

void Physics::desync() {}

void Physics::destroy() {}

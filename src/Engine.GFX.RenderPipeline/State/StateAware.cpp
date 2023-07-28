#include "StateAware.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

void StateAware::regSubState(nmpt<State> state_) const {}

bool StateAware::isReallocSupported() const noexcept {
    return false;
}

bool StateAware::realloc(nmpt<State> state_) {
    return false;
}

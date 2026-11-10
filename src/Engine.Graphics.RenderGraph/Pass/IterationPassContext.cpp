#include "IterationPassContext.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

ref<SymbolContext> IterationPassContext::symbols() const noexcept {
    return _symbolContext;
}

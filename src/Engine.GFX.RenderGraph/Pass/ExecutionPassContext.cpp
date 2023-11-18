#include "ExecutionPassContext.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

cref<SymbolContext> ExecutionPassContext::symbols() const noexcept {
    return _symbolContext;
}

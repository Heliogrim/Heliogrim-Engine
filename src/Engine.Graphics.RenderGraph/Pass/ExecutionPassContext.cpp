#include "ExecutionPassContext.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

cref<SymbolContext> ExecutionPassContext::symbols() const noexcept {
    return _symbolContext;
}

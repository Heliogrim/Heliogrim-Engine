#include "ResolvePassContext.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

ref<SymbolContext> ResolvePassContext::symbols() const noexcept {
    return _symbolContext;
}

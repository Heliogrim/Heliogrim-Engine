#pragma once
#include "SymbolContext.hpp"

namespace hg::engine::render::graph {
    class ScopedSymbolContext :
        public SymbolContext {
    public:
        using SymbolContext::SymbolContext;
    };
}

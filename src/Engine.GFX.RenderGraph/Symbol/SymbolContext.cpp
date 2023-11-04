#include "SymbolContext.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

void SymbolContext::exposeSymbol(mref<smr<const acc::Symbol>> symbol_, nmpt<SymbolizedResource> resource_) {}

nmpt<SymbolizedResource> SymbolContext::exportSymbol(mref<smr<const acc::Symbol>> symbol_) {
    return nullptr;
}

nmpt<SymbolizedResource> SymbolContext::importSymbol(mref<smr<const acc::Symbol>> symbol_) {
    return nullptr;
}

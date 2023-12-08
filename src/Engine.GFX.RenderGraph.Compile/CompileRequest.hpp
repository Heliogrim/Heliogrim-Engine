#pragma once
#include <Engine.GFX.RenderGraph/CompileGraph.hpp>

namespace hg::engine::render::graph {
    struct CompileRequest final {
        smr<CompileGraph> graph;
        /**/
        Vector<smr<const Symbol>> targetSymbols;
    };
}

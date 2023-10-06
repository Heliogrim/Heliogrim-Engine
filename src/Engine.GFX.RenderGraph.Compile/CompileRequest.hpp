#pragma once
#include <Engine.GFX.RenderGraph/CompileGraph.hpp>

namespace hg::engine::gfx::render::graph {
    struct CompileRequest final {
        smr<CompileGraph> graph;
        /**/
        Vector<smr<const acc::Symbol>> targetSymbols;
    };
}

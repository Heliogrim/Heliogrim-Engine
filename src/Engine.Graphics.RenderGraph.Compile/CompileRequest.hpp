#pragma once
#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Graphics.RenderGraph;
#endif

namespace hg::engine::render::graph {
    struct CompileRequest final {
        smr<CompileGraph> graph;
        /**/
        Vector<smr<const Symbol>> targetSymbols;
    };
}

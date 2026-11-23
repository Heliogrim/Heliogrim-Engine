#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Graphics.RenderGraph;
#endif

namespace hg::engine::render {
    [[nodiscard]] extern smr<const graph::Symbol> makeSceneStencilSymbol();
}

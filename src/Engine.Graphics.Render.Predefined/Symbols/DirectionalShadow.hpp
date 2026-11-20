#pragma once
#include <Engine.Graphics.RenderGraph/Symbol/Symbol.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render {
    [[nodiscard]] extern smr<const graph::Symbol> makeDirectionalShadowSymbol();
}

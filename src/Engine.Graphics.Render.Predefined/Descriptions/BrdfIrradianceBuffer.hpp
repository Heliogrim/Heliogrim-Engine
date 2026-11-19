#pragma once
#include <Engine.Common/__macro.hpp>
#include <Engine.Graphics.RenderGraph/Relation/Description.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render {
    [[nodiscard]] extern smr<const graph::Description> makeBrdfIrradianceBufferDescription();
}

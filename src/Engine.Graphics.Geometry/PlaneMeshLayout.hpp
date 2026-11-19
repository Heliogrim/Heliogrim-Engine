#pragma once
#include <Engine.Common/__macro.hpp>
#include <Engine.Graphics.RenderGraph/Relation/MeshDescription.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    [[nodiscard]] extern smr<render::graph::MeshDescription> GetPlaneMeshLayout();
}

inline hg::smr<hg::engine::render::graph::MeshDescription> hg::engine::graphics::GetPlaneMeshLayout() {
    static smr<render::graph::MeshDescription> planeMeshLayout {};
    return planeMeshLayout;
}

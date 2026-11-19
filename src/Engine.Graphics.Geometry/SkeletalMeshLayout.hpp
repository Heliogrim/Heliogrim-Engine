#pragma once
#include <Engine.Common/__macro.hpp>
#include <Engine.Graphics.RenderGraph/Relation/MeshDescription.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    [[nodiscard]] extern smr<render::graph::MeshDescription> GetSkeletalMeshLayout();
}

inline hg::smr<hg::engine::render::graph::MeshDescription> hg::engine::graphics::GetSkeletalMeshLayout() {
    static smr<render::graph::MeshDescription> skeletalMeshLayout {};
    return skeletalMeshLayout;
}

#pragma once
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.RenderGraph/Relation/MeshDescription.hpp>

namespace hg::engine::gfx {
    [[nodiscard]] extern smr<render::graph::MeshDescription> GetPlaneMeshLayout();
}

inline hg::smr<hg::engine::render::graph::MeshDescription> hg::engine::gfx::GetPlaneMeshLayout() {
    static smr<render::graph::MeshDescription> planeMeshLayout {};
    return planeMeshLayout;
}

#pragma once
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.RenderGraph/Relation/MeshDescription.hpp>

namespace hg::engine::gfx {
    [[nodiscard]] extern smr<render::graph::MeshDescription> GetStaticMeshLayout();
}

inline hg::smr<hg::engine::gfx::render::graph::MeshDescription> hg::engine::gfx::GetStaticMeshLayout() {
    static smr<render::graph::MeshDescription> staticMeshLayout {};
    return staticMeshLayout;
}

#pragma once
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Graphics.RenderGraph/Relation/MeshDescription.hpp>

namespace hg::engine::graphics {
    [[nodiscard]] extern smr<render::graph::MeshDescription> GetStaticMeshLayout();
}

inline hg::smr<hg::engine::render::graph::MeshDescription> hg::engine::graphics::GetStaticMeshLayout() {
    static smr<render::graph::MeshDescription> staticMeshLayout {};
    return staticMeshLayout;
}

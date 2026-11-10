#pragma once
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Graphics.RenderGraph/Relation/MeshDescription.hpp>

namespace hg::engine::graphics {
    [[nodiscard]] extern smr<render::graph::MeshDescription> GetSkeletalMeshLayout();
}

inline hg::smr<hg::engine::render::graph::MeshDescription> hg::engine::graphics::GetSkeletalMeshLayout() {
    static smr<render::graph::MeshDescription> skeletalMeshLayout {};
    return skeletalMeshLayout;
}

#pragma once
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.RenderGraph/Relation/MeshDescription.hpp>

namespace hg::engine::gfx {
    [[nodiscard]] extern smr<render::graph::MeshDescription> GetSkeletalMeshLayout();
}

inline hg::smr<hg::engine::gfx::render::graph::MeshDescription> hg::engine::gfx::GetSkeletalMeshLayout() {
    static smr<render::graph::MeshDescription> skeletalMeshLayout {};
    return skeletalMeshLayout;
}

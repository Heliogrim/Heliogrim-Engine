#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "../RenderCommand.hpp"

namespace hg::engine::gfx {
    class Mesh;
}

namespace hg::engine::gfx::render::cmd {
    class BindSkeletalMeshRenderCommand :
        public RenderCommand {
    public:
        using this_type = BindSkeletalMeshRenderCommand;

    public:
        constexpr BindSkeletalMeshRenderCommand(mref<const ptr<const Mesh>> skeletalMesh_) noexcept :
            RenderCommand(),
            _skeletalMesh(_STD move(skeletalMesh_)) {}

        constexpr ~BindSkeletalMeshRenderCommand() noexcept = default;

    private:
        const ptr<const Mesh> _skeletalMesh;

    public:
        void operator()(ptr<RenderCommandTranslationUnit> rctu_) noexcept override;
    };
}

#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "../RenderCommand.hpp"

namespace hg::engine::gfx {
    class Mesh;
}

namespace hg::engine::render::cmd {
    class BindSkeletalMeshRCmd :
        public RenderCommand {
    public:
        using this_type = BindSkeletalMeshRCmd;

    public:
        constexpr BindSkeletalMeshRCmd(mref<const nmpt<const gfx::Mesh>> skeletalMesh_) noexcept :
            RenderCommand(),
            _skeletalMesh(std::move(skeletalMesh_)) {}

        constexpr ~BindSkeletalMeshRCmd() noexcept override = default;

    private:
    public:
        const nmpt<const gfx::Mesh> _skeletalMesh;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}

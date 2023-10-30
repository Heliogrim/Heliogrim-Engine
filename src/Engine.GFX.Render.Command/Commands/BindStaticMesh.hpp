#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "../RenderCommand.hpp"
#include "__fwd.hpp"

namespace hg::engine::gfx::render::cmd {
    class BindStaticMeshRenderCommand :
        public RenderCommand {
    public:
        using this_type = BindStaticMeshRenderCommand;

    public:
        constexpr BindStaticMeshRenderCommand(mref<const ptr<const Mesh>> staticMesh_) noexcept :
            RenderCommand(),
            _staticMesh(_STD move(staticMesh_)) {}

        constexpr ~BindStaticMeshRenderCommand() noexcept = default;

    private:
    public:
        const ptr<const Mesh> _staticMesh;

    public:
        void operator()(ptr<RenderCommandTranslationUnit> rctu_) const noexcept override;
    };
}

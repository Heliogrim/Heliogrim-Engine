#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "../RenderCommand.hpp"
#include "__fwd.hpp"

namespace hg::engine::render::cmd {
    class BindStaticMeshRCmd :
        public RenderCommand {
    public:
        using this_type = BindStaticMeshRCmd;

    public:
        constexpr BindStaticMeshRCmd(mref<const nmpt<const gfx::Mesh>> staticMesh_) noexcept :
            RenderCommand(),
            _staticMesh(_STD move(staticMesh_)) {}

        constexpr ~BindStaticMeshRCmd() noexcept override = default;

    private:
    public:
        const nmpt<const gfx::Mesh> _staticMesh;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}

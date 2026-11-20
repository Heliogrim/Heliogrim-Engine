#pragma once

#include "../RenderCommand.hpp"
#include "__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render::cmd {
    class BindStaticMeshRCmd :
        public RenderCommand {
    public:
        using this_type = BindStaticMeshRCmd;

    public:
        constexpr BindStaticMeshRCmd(mref<const nmpt<const graphics::Mesh>> staticMesh_) noexcept :
            RenderCommand(),
            _staticMesh(std::move(staticMesh_)) {}

        constexpr ~BindStaticMeshRCmd() noexcept override = default;

    private:
    public:
        const nmpt<const graphics::Mesh> _staticMesh;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}

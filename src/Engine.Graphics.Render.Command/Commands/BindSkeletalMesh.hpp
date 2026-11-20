#pragma once

#include "../RenderCommand.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    class Mesh;
}

namespace hg::engine::render::cmd {
    class BindSkeletalMeshRCmd :
        public RenderCommand {
    public:
        using this_type = BindSkeletalMeshRCmd;

    public:
        constexpr BindSkeletalMeshRCmd(mref<const nmpt<const graphics::Mesh>> skeletalMesh_) noexcept :
            RenderCommand(),
            _skeletalMesh(std::move(skeletalMesh_)) {}

        constexpr ~BindSkeletalMeshRCmd() noexcept override = default;

    private:
    public:
        const nmpt<const graphics::Mesh> _skeletalMesh;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}

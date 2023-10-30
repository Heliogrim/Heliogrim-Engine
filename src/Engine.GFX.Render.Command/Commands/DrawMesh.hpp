#pragma once
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include <Engine.GFX.RenderGraph/Relation/MeshDescription.hpp>
#include "../RenderCommand.hpp"

namespace hg::engine::gfx::render::cmd {
    class DrawMeshRenderCommand :
        public RenderCommand {
    public:
        using this_type = DrawMeshRenderCommand;

    public:
        DrawMeshRenderCommand(
            const ptr<const graph::MeshDescription> meshDescription_,
            const u32 instanceCount_,
            const u32 instanceOffset_,
            const u32 primitiveCount_,
            const u32 primitiveOffset_,
            const bool indexedPrimitive_
        ) noexcept :
            RenderCommand(),
            _meshDescription(meshDescription_),
            _instanceCount(instanceCount_),
            _instanceOffset(instanceOffset_),
            _primitiveCount(primitiveCount_),
            _primitiveOffset(primitiveOffset_),
            _indexedPrimitive(indexedPrimitive_) {}

        constexpr ~DrawMeshRenderCommand() noexcept = default;

    private:
    public:
        const ptr<const graph::MeshDescription> _meshDescription;
        const u32 _instanceCount;
        const u32 _instanceOffset;
        const u32 _primitiveCount;
        const u32 _primitiveOffset;
        const bool _indexedPrimitive;

    public:
        void operator()(ptr<RenderCommandTranslationUnit> rctu_) const noexcept override;
    };
}

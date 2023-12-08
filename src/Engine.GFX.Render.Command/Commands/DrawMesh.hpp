#pragma once
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include <Engine.GFX.RenderGraph/Relation/MeshDescription.hpp>
#include "../RenderCommand.hpp"

namespace hg::engine::render::cmd {
    class DrawMeshRCmd :
        public RenderCommand {
    public:
        using this_type = DrawMeshRCmd;

    public:
        DrawMeshRCmd(
            const nmpt<const render::graph::MeshDescription> meshDescription_,
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

        constexpr ~DrawMeshRCmd() noexcept override = default;

    private:
    public:
        const nmpt<const render::graph::MeshDescription> _meshDescription;
        const u32 _instanceCount;
        const u32 _instanceOffset;
        const u32 _primitiveCount;
        const u32 _primitiveOffset;
        const bool _indexedPrimitive;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}

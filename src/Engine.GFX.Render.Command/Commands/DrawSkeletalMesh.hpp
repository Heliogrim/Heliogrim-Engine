#pragma once
#include <Engine.Common/Types.hpp>

#include "Engine.GFX.Render.Command/RenderCommand.hpp"

namespace hg::engine::gfx::render::cmd {
    class DrawSkeletalMeshRenderCommand :
        public RenderCommand {
    public:
        using this_type = DrawSkeletalMeshRenderCommand;

    public:
        constexpr DrawSkeletalMeshRenderCommand(
            const u32 instanceCount_,
            const u32 instanceOffset_,
            const u32 primitiveCount_,
            const u32 primitiveOffset_,
            const bool indexedPrimitive_
        ) noexcept :
            RenderCommand(),
            _instanceCount(instanceCount_),
            _instanceOffset(instanceOffset_),
            _primitiveCount(primitiveCount_),
            _primitiveOffset(primitiveOffset_),
            _indexedPrimitive(indexedPrimitive_) {}

        constexpr ~DrawSkeletalMeshRenderCommand() noexcept = default;

    private:
        const u32 _instanceCount;
        const u32 _instanceOffset;
        const u32 _primitiveCount;
        const u32 _primitiveOffset;
        const bool _indexedPrimitive;

    public:
        void operator()(ptr<RenderCommandTranslationUnit> rctu_) const noexcept override;
    };
}

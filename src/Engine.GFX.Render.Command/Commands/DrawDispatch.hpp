#pragma once
#include <Engine.Common/Types.hpp>

#include "../RenderCommand.hpp"

namespace hg::engine::gfx::render::cmd {
    class DrawDispatchRenderCommand final :
        public RenderCommand {
    public:
        using this_type = DrawDispatchRenderCommand;

    public:
        constexpr DrawDispatchRenderCommand(
            const u32 instanceCount_,
            const u32 instanceOffset_,
            const u32 primitiveCount_,
            const u32 primitiveOffset_
        ) noexcept :
            _instanceCount(instanceCount_),
            _instanceOffset(instanceOffset_),
            _primitiveCount(primitiveCount_),
            _primitiveOffset(primitiveOffset_) {}

        constexpr ~DrawDispatchRenderCommand() noexcept = default;

    private:
        const u32 _instanceCount;
        const u32 _instanceOffset;
        const u32 _primitiveCount;
        const u32 _primitiveOffset;

    public:
        void operator()(ptr<RenderCommandTranslationUnit> rctu_) const noexcept override;
    };
}

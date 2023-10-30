#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "Engine.GFX.Render.Command/RenderCommand.hpp"

namespace hg::engine::gfx {
    class VertexBufferView;
}

namespace hg::engine::gfx::render::cmd {
    class BindVertexBufferRenderCommand :
        public RenderCommand {
    public:
        constexpr BindVertexBufferRenderCommand(
            mref<const ptr<const VertexBufferView>> vertexView_
        ) noexcept :
            RenderCommand(),
            _vertexView(_STD move(vertexView_)) {}

        constexpr ~BindVertexBufferRenderCommand() noexcept = default;

    private:
    public:
        const ptr<const VertexBufferView> _vertexView;

    public:
        void operator()(ptr<RenderCommandTranslationUnit> rctu_) const noexcept override;
    };
}

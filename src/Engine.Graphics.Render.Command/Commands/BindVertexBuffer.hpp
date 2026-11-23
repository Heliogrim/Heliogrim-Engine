#pragma once

#include "Engine.Graphics.Render.Command/RenderCommand.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Graphics;
#endif

namespace hg::engine::render::cmd {
    class BindVertexBufferRCmd :
        public RenderCommand {
    public:
        constexpr BindVertexBufferRCmd(
            cref<graphics::VertexBufferView> vertexBufferView_
        ) noexcept :
            RenderCommand(),
            _vertexBufferView(vertexBufferView_) {}

        constexpr ~BindVertexBufferRCmd() noexcept override = default;

    private:
    public:
        graphics::VertexBufferView _vertexBufferView;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}

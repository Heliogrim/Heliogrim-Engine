#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "Engine.Graphics.Render.Command/RenderCommand.hpp"
#include <Engine.Graphics/Buffer/VertexBufferView.hpp>

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

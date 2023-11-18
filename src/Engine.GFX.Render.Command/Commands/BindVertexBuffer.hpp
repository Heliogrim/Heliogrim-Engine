#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "Engine.GFX.Render.Command/RenderCommand.hpp"
#include <Engine.GFX/Buffer/VertexBufferView.hpp>

namespace hg::engine::gfx::render::cmd {
    class BindVertexBufferRCmd :
        public RenderCommand {
    public:
        constexpr BindVertexBufferRCmd(
            cref<VertexBufferView> vertexBufferView_
        ) noexcept :
            RenderCommand(),
            _vertexBufferView(vertexBufferView_) {}

        constexpr ~BindVertexBufferRCmd() noexcept override = default;

    private:
    public:
        VertexBufferView _vertexBufferView;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}

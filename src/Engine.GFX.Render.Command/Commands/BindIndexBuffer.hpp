#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "Engine.GFX.Render.Command/RenderCommand.hpp"
#include <Engine.GFX/Buffer/IndexBufferView.hpp>

namespace hg::engine::render::cmd {
    class BindIndexBufferRCmd :
        public RenderCommand {
    public:
        constexpr BindIndexBufferRCmd(
            cref<gfx::IndexBufferView> indexBufferView_
        ) noexcept :
            RenderCommand(),
            _indexBufferView(indexBufferView_) {}

        constexpr ~BindIndexBufferRCmd() noexcept override = default;

    private:
    public:
        gfx::IndexBufferView _indexBufferView;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}

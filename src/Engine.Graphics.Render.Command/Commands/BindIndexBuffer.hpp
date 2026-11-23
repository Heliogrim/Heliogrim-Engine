#pragma once

#include "Engine.Graphics.Render.Command/RenderCommand.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Graphics;
#endif

namespace hg::engine::render::cmd {
    class BindIndexBufferRCmd :
        public RenderCommand {
    public:
        constexpr BindIndexBufferRCmd(
            cref<graphics::IndexBufferView> indexBufferView_
        ) noexcept :
            RenderCommand(),
            _indexBufferView(indexBufferView_) {}

        constexpr ~BindIndexBufferRCmd() noexcept override = default;

    private:
    public:
        graphics::IndexBufferView _indexBufferView;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}

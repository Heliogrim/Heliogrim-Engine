#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "Engine.GFX.Render.Command/RenderCommand.hpp"

namespace hg::engine::gfx {
    class IndexBufferView;
}

namespace hg::engine::gfx::render::cmd {
    class BindIndexBufferRenderCommand :
        public RenderCommand {
    public:
        constexpr BindIndexBufferRenderCommand(
            mref<const ptr<const IndexBufferView>> indexView_
        ) noexcept :
            RenderCommand(),
            _indexView(_STD move(indexView_)) {}

        constexpr ~BindIndexBufferRenderCommand() noexcept = default;

    private:
    public:
        const ptr<const IndexBufferView> _indexView;

    public:
        void operator()(ptr<RenderCommandTranslationUnit> rctu_) const noexcept override;
    };
}

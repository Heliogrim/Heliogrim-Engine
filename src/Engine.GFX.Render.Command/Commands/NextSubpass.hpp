#pragma once
#include "Engine.GFX.Render.Command/RenderCommand.hpp"

namespace hg::engine::gfx::render::cmd {
    class NextSubpassRenderCommand :
        public RenderCommand {
    public:
        constexpr NextSubpassRenderCommand() noexcept = default;

        constexpr ~NextSubpassRenderCommand() noexcept = default;

    public:
        void operator()(ptr<RenderCommandTranslationUnit> rctu_) noexcept override;
    };
}

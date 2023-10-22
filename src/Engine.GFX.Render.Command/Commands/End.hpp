#pragma once
#include "Engine.GFX.Render.Command/RenderCommand.hpp"

namespace hg::engine::gfx::render::cmd {
    class EndRenderCommand :
        public RenderCommand {
    public:
        constexpr EndRenderCommand() noexcept = default;

        constexpr ~EndRenderCommand() noexcept = default;

    public:
        void operator()(ptr<RenderCommandTranslationUnit> rctu_) noexcept override;
    };
}

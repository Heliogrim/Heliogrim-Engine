#pragma once
#include "Engine.GFX.Render.Command/RenderCommand.hpp"

namespace hg::engine::gfx::render::cmd {
    class BeginRenderCommand :
        public RenderCommand {
    public:
        constexpr BeginRenderCommand() noexcept = default;

        constexpr ~BeginRenderCommand() noexcept = default;

    public:
        void operator()(ptr<RenderCommandTranslationUnit> rctu_) noexcept override;
    };
}

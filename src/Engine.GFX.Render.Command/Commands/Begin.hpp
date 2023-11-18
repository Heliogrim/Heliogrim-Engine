#pragma once
#include "Engine.GFX.Render.Command/RenderCommand.hpp"

namespace hg::engine::gfx::render::cmd {
    class BeginRCmd :
        public RenderCommand {
    public:
        constexpr BeginRCmd() noexcept = default;

        constexpr ~BeginRCmd() noexcept override = default;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}

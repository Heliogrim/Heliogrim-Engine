#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "Engine.GFX.Render.Command/RenderCommand.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationEffect;
}

namespace hg::engine::gfx::render::cmd {
    class BindEffectRenderCommand :
        public RenderCommand {
    public:
        constexpr BindEffectRenderCommand(
            mref<const ptr<const acc::AccelerationEffect>> effect_
        ) noexcept :
            RenderCommand(),
            _effect(_STD move(effect_)) {}

        constexpr ~BindEffectRenderCommand() noexcept = default;

    private:
    public:
        const ptr<const acc::AccelerationEffect> _effect;

    public:
        void operator()(ptr<RenderCommandTranslationUnit> rctu_) const noexcept override;
    };
}

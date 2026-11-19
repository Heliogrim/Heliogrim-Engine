#pragma once
#include <Engine.Accel.Lang/Rep/Symbol.hpp>
#include <Engine.Common/__macro.hpp>

#include "../RenderCommand.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    class TextureSampler;
}

namespace hg::engine::render::cmd {
    class BindTextureSamplerRCmd :
        public RenderCommand {
    public:
        constexpr BindTextureSamplerRCmd(
            mref<accel::lang::SymbolId> symbolId_,
            mref<nmpt<const graphics::TextureSampler>> textureSampler_
        ) noexcept :
            RenderCommand(),
            _symbolId(std::move(symbolId_)),
            _textureSampler(std::move(textureSampler_)) {}

        constexpr ~BindTextureSamplerRCmd() noexcept override = default;

    private:
    public:
        accel::lang::SymbolId _symbolId;
        const nmpt<const graphics::TextureSampler> _textureSampler;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}

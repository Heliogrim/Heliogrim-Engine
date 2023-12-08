#pragma once
#include <Engine.Accel.Lang/Rep/Symbol.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "../RenderCommand.hpp"

namespace hg::engine::gfx {
    class SampledTextureView;
}

namespace hg::engine::render::cmd {
    class BindTextureRCmd :
        public RenderCommand {
    public:
        constexpr BindTextureRCmd(
            mref<accel::lang::SymbolId> symbolId_,
            mref<const nmpt<const gfx::SampledTextureView>> textureView_
        ) noexcept :
            RenderCommand(),
            _symbolId(_STD move(symbolId_)),
            _sampledTexture(_STD move(textureView_)) {}

        constexpr ~BindTextureRCmd() noexcept override = default;

    private:
    public:
        accel::lang::SymbolId _symbolId;
        const nmpt<const gfx::SampledTextureView> _sampledTexture;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}

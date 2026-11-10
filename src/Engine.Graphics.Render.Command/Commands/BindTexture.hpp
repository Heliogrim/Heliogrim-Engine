#pragma once
#include <Engine.Accel.Lang/Rep/Symbol.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "../RenderCommand.hpp"

namespace hg::engine::gfx {
    class TextureLikeObject;
}

namespace hg::engine::render::cmd {
    class BindTextureRCmd :
        public RenderCommand {
    public:
        constexpr BindTextureRCmd(
            mref<accel::lang::SymbolId> symbolId_,
            mref<nmpt<const gfx::TextureLikeObject>> textureView_
        ) noexcept :
            RenderCommand(),
            _symbolId(std::move(symbolId_)),
            _textureView(std::move(textureView_)) {}

        constexpr ~BindTextureRCmd() noexcept override = default;

    private:
    public:
        accel::lang::SymbolId _symbolId;
        const nmpt<const gfx::TextureLikeObject> _textureView;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}

#pragma once
#include <Engine.Accel.Lang/Rep/Symbol.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Engine.GFX.Render.Command/RenderCommand.hpp"

namespace hg::engine::gfx {
    class UniformBufferView;
}

namespace hg::engine::render::cmd {
    class BindUniformBufferRCmd :
        public RenderCommand {
    public:
        constexpr BindUniformBufferRCmd(
            mref<accel::lang::SymbolId> symbolId_,
            mref<const nmpt<const gfx::UniformBufferView>> uniformView_
        ) noexcept :
            RenderCommand(),
            _symbolId(std::move(symbolId_)),
            _uniformView(std::move(uniformView_)) {}

        constexpr ~BindUniformBufferRCmd() noexcept override = default;

    private:
    public:
        const accel::lang::SymbolId _symbolId;
        const nmpt<const gfx::UniformBufferView> _uniformView;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}

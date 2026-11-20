#pragma once
#include <Engine.Accel.Lang/Rep/Symbol.hpp>

#include "Engine.Graphics.Render.Command/RenderCommand.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    class UniformBufferView;
}

namespace hg::engine::render::cmd {
    class BindUniformBufferRCmd :
        public RenderCommand {
    public:
        constexpr BindUniformBufferRCmd(
            mref<accel::lang::SymbolId> symbolId_,
            mref<const nmpt<const graphics::UniformBufferView>> uniformView_
        ) noexcept :
            RenderCommand(),
            _symbolId(std::move(symbolId_)),
            _uniformView(std::move(uniformView_)) {}

        constexpr ~BindUniformBufferRCmd() noexcept override = default;

    private:
    public:
        const accel::lang::SymbolId _symbolId;
        const nmpt<const graphics::UniformBufferView> _uniformView;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}

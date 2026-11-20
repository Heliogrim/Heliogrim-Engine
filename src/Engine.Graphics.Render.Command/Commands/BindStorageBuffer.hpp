#pragma once
#include <Engine.Accel.Lang/Rep/Symbol.hpp>

#include "Engine.Graphics.Render.Command/RenderCommand.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    class StorageBufferView;
}

namespace hg::engine::render::cmd {
    class BindStorageBufferRCmd :
        public RenderCommand {
    public:
        constexpr BindStorageBufferRCmd(
            mref<accel::lang::SymbolId> symbolId_,
            mref<const nmpt<const graphics::StorageBufferView>> storageView_
        ) noexcept :
            RenderCommand(),
            _symbolId(std::move(symbolId_)),
            _storageView(std::move(storageView_)) {}

        constexpr ~BindStorageBufferRCmd() noexcept override = default;

    private:
    public:
        const accel::lang::SymbolId _symbolId;
        const nmpt<const graphics::StorageBufferView> _storageView;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}

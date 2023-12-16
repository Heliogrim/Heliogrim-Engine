#pragma once
#include <Engine.Accel.Lang/Rep/Symbol.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Engine.GFX.Render.Command/RenderCommand.hpp"

namespace hg::engine::gfx {
    class StorageBufferView;
}

namespace hg::engine::render::cmd {
    class BindStorageBufferRCmd :
        public RenderCommand {
    public:
        constexpr BindStorageBufferRCmd(
            mref<accel::lang::SymbolId> symbolId_,
            mref<const nmpt<const gfx::StorageBufferView>> storageView_
        ) noexcept :
            RenderCommand(),
            _symbolId(_STD move(symbolId_)),
            _storageView(_STD move(storageView_)) {}

        constexpr ~BindStorageBufferRCmd() noexcept override = default;

    private:
    public:
        const accel::lang::SymbolId _symbolId;
        const nmpt<const gfx::StorageBufferView> _storageView;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}

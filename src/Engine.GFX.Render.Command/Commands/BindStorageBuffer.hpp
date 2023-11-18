#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Engine.GFX.Render.Command/RenderCommand.hpp"

namespace hg::engine::gfx {
    class StorageBufferView;
}

namespace hg::engine::gfx::render::cmd {
    class BindStorageBufferRCmd :
        public RenderCommand {
    public:
        constexpr BindStorageBufferRCmd(
            mref<const nmpt<const StorageBufferView>> storageView_
        ) noexcept :
            RenderCommand(),
            _storageView(_STD move(storageView_)) {}

        constexpr ~BindStorageBufferRCmd() noexcept override = default;

    private:
        const nmpt<const StorageBufferView> _storageView;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}

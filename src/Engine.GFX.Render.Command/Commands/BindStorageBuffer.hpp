#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "Engine.GFX.Render.Command/RenderCommand.hpp"

namespace hg::engine::gfx {
    class StorageBufferView;
}

namespace hg::engine::gfx::render::cmd {
    class BindStorageBufferRenderCommand :
        public RenderCommand {
    public:
        constexpr BindStorageBufferRenderCommand(
            mref<const ptr<const StorageBufferView>> storageView_
        ) noexcept :
            RenderCommand(),
            _storageView(_STD move(storageView_)) {}

        constexpr ~BindStorageBufferRenderCommand() noexcept = default;

    private:
        const ptr<const StorageBufferView> _storageView;

    public:
        void operator()(ptr<RenderCommandTranslationUnit> rctu_) const noexcept override;
    };
}

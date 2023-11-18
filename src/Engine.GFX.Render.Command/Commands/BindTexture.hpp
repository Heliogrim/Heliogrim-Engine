#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Engine.GFX.Render.Command/RenderCommand.hpp"

namespace hg::engine::gfx {
    class TextureView;
    class VirtualTextureView;
}

namespace hg::engine::gfx::render::cmd {
    namespace {
        struct VirtualTextureBinding {
            const bool vt;
            const nmpt<const VirtualTextureView> view;
        };

        struct OpaqueTextureBinding {
            const bool vt;
            const nmpt<const TextureView> view;
        };
    }

    class BindTextureRCmd :
        public RenderCommand {
    public:
        constexpr BindTextureRCmd(
            mref<const nmpt<const TextureView>> textureView_
        ) noexcept :
            RenderCommand(),
            _texture(decltype(_texture) { .otb = OpaqueTextureBinding { false, _STD move(textureView_) } }) {}

        constexpr BindTextureRCmd(
            mref<const nmpt<const VirtualTextureView>> textureView_
        ) noexcept :
            RenderCommand(),
            _texture(decltype(_texture) { .vtb = VirtualTextureBinding { true, _STD move(textureView_) } }) {}

        constexpr ~BindTextureRCmd() noexcept override = default;

    private:
        union {
            VirtualTextureBinding vtb;
            OpaqueTextureBinding otb;
        } _texture;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}

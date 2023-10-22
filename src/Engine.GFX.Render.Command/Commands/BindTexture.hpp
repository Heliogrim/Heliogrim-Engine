#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "Engine.GFX.Render.Command/RenderCommand.hpp"

namespace hg::engine::gfx {
    class TextureView;
    class VirtualTextureView;
}

namespace hg::engine::gfx::render::cmd {
    namespace {
        struct VirtualTextureBinding {
            const bool vt;
            const ptr<const VirtualTextureView> view;
        };

        struct OpaqueTextureBinding {
            const bool vt;
            const ptr<const TextureView> view;
        };
    }

    class BindTextureRenderCommand :
        public RenderCommand {
    public:
        constexpr BindTextureRenderCommand(
            mref<const ptr<const TextureView>> textureView_
        ) noexcept :
            RenderCommand(),
            _texture({ .otb = OpaqueTextureBinding { false, _STD move(textureView_) } }) {}

        constexpr BindTextureRenderCommand(
            mref<const ptr<const VirtualTextureView>> textureView_
        ) noexcept :
            RenderCommand(),
            _texture({ .vtb = VirtualTextureBinding { true, _STD move(textureView_) } }) {}

        constexpr ~BindTextureRenderCommand() noexcept = default;

    private:
        union {
            VirtualTextureBinding vtb;
            OpaqueTextureBinding otb;
        } _texture;

    public:
        void operator()(ptr<RenderCommandTranslationUnit> rctu_) noexcept override;
    };
}

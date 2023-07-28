#pragma once

#include <Engine.GFX.Glow.3D/Renderer/RevRenderer.hpp>

namespace hg::editor::gfx {
    class EdRevRenderer :
        public ::hg::engine::gfx::glow::render::RevRenderer {
    public:
        using this_type = EdRevRenderer;
        using underlying_type = ::hg::engine::gfx::glow::render::RevRenderer;
        using base_type = Renderer_Deprecated;

    public:
        EdRevRenderer();

        ~EdRevRenderer() override = default;

    public:
        void setup(cref<sptr<engine::gfx::Device>> device_) override;

        void destroy() override;

    protected:
        void registerStages() override;
    };
}

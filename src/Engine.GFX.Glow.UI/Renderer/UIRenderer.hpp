#pragma once
#include <Engine.GFX.Renderer/Renderer_Deprecated.hpp>

namespace hg::engine::gfx::glow::ui::render {
    class UiRenderer :
        public gfx::render::Renderer_Deprecated {
    public:
        using this_type = UiRenderer;
        using base_type = Renderer_Deprecated;

    public:
        UiRenderer();

        ~UiRenderer() override = default;

    public:
        void setup(cref<sptr<Device>> device_) override;

        void destroy() override;
    };
}

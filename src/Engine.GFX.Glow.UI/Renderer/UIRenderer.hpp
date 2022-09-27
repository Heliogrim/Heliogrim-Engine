#pragma once
#include <Engine.GFX/Renderer/Renderer.hpp>

namespace ember::engine::gfx::glow::ui::render {

    class UiRenderer :
        public gfx::render::Renderer {
    public:
        using this_type = UiRenderer;
        using base_type = Renderer;

    public:
        UiRenderer();

        ~UiRenderer() override = default;

    public:
        void setup(cref<sptr<Device>> device_) override;

        void destroy() override;
    };

}

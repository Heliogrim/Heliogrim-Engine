#pragma once
#include <Engine.GFX/Renderer/RenderStage.hpp>

namespace ember::engine::gfx::glow::ui::render {
    class UiMainStage final :
        public gfx::render::RenderStage {
    public:
        using this_type = UiMainStage;

    public:
        UiMainStage() = default;

        ~UiMainStage() override = default;

    protected:
        void setup(cref<sptr<Device>> device_) override;

        void destroy() override;
    };
}

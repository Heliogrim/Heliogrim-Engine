#pragma once
#include <Engine.GFX/Renderer/RenderStage.hpp>

namespace ember::engine::gfx::glow::render {

    class RevFinalStage :
        public gfx::render::RenderStage {
    public:
        using this_type = RevFinalStage;

    public:
        RevFinalStage() = default;

        ~RevFinalStage() override = default;

    protected:
        void setup(cref<sptr<Device>> device_) override;

        void destroy() override;
    };
}

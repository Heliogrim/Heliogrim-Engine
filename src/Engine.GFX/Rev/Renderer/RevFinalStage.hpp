#pragma once
#include <Engine.GFX/Renderer/RenderStage.hpp>

namespace ember::engine::gfx::render {

    class RevFinalStage :
        public RenderStage {
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

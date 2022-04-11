#pragma once
#include <Engine.GFX/Renderer/RenderStage.hpp>

namespace ember::engine::gfx::render {

    class RevDepthStage :
        public RenderStage {
    public:
        using this_type = RevDepthStage;

    public:
        RevDepthStage() = default;

        ~RevDepthStage() override = default;

    protected:
        void setup(cref<sptr<Device>> device_) override;

        void destroy() override;
    };

}

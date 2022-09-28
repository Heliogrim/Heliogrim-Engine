#pragma once
#include <Engine.GFX/Renderer/RenderStage.hpp>

namespace ember::engine::gfx::render {

    class RevEarlyStage final :
        public RenderStage {
    public:
        using this_type = RevEarlyStage;

    public:
        RevEarlyStage() = default;

        ~RevEarlyStage() override = default;

    protected:
        void setup(cref<sptr<Device>> device_) override;

        void destroy() override;
    };

}

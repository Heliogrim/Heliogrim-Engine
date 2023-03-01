#pragma once
#include <Engine.GFX/Renderer/RenderStage.hpp>

namespace hg::engine::gfx::glow::render {
    class RevEarlyStage final :
        public gfx::render::RenderStage {
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

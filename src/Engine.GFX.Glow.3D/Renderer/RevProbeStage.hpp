#pragma once
#include <Engine.GFX/Renderer/RenderStage.hpp>

namespace ember::engine::gfx::glow::render {
    class RevProbeStage :
        public gfx::render::RenderStage {
    public:
        using this_type = RevProbeStage;

    public:
        RevProbeStage() = default;

        ~RevProbeStage() override = default;

    protected:
        void setup(cref<sptr<Device>> device_) override;

        void destroy() override;

    public:
        bool allocate(const ptr<gfx::render::HORenderPass> renderPass_) override;

        void free(const ptr<gfx::render::HORenderPass> renderPass_) override;
    };
}

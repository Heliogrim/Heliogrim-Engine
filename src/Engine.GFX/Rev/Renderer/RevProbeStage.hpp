#pragma once
#include <Engine.GFX/Renderer/RenderStage.hpp>

namespace ember::engine::gfx::render {

    class RevProbeStage :
        public RenderStage {
    public:
        using this_type = RevProbeStage;

    public:
        RevProbeStage() = default;

        ~RevProbeStage() override = default;

    protected:
        void setup(cref<sptr<Device>> device_) override;

        void destroy() override;

    public:
        bool allocate(const ptr<HORenderPass> renderPass_) override;

        void free(const ptr<HORenderPass> renderPass_) override;
    };
}

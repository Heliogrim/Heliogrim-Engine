#pragma once
#include <Engine.GFX.Renderer/Renderer_Deprecated.hpp>

namespace hg::engine::gfx::glow::render {
    class RevRenderer :
        public gfx::render::Renderer_Deprecated {
    public:
        using this_type = RevRenderer;
        using base_type = Renderer_Deprecated;

    public:
        RevRenderer();

        ~RevRenderer() override = default;

    public:
        void setup(cref<sptr<Device>> device_) override;

        void destroy() override;

    protected:
        virtual void registerStages();
    };
}

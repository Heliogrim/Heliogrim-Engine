#pragma once
#include <Engine.GFX.Renderer/Renderer.hpp>

namespace hg::engine::gfx::glow::render {
    class RevRenderer :
        public gfx::render::Renderer {
    public:
        using this_type = RevRenderer;
        using base_type = Renderer;

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

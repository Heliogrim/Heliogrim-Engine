#pragma once
#include <Engine.GFX/Renderer/RenderStage.hpp>

namespace hg::engine::gfx::glow::render {
    class RevMainStage final :
        public gfx::render::RenderStage {
    public:
        using this_type = RevMainStage;

    public:
        RevMainStage() = default;

        ~RevMainStage() override = default;

    protected:
        void setup(cref<sptr<Device>> device_) override;

        void destroy() override;
    };
}

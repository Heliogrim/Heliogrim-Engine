#pragma once
#include <Engine.GFX/Renderer/RenderStage.hpp>

namespace ember::engine::gfx::render {

    class RevMainStage final :
        public RenderStage {
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

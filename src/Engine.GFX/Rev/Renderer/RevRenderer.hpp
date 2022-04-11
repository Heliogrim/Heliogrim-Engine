#pragma once
#include "../../Renderer/Renderer.hpp"

namespace ember::engine::gfx::render {

    class RevRenderer :
        public Renderer {
    public:
        using this_type = RevRenderer;
        using base_type = Renderer;

    public:
        RevRenderer();

        ~RevRenderer() override = default;

    public:
        void setup(cref<sptr<Device>> device_) override;

        void destroy() override;
    };
}

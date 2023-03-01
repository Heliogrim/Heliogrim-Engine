#pragma once
#include <Engine.GFX/Renderer/RenderStage.hpp>

namespace hg::editor::gfx {
    class EdPostCompStage :
        public engine::gfx::render::RenderStage {
    public:
        using this_type = EdPostCompStage;

    public:
        EdPostCompStage() = default;

        ~EdPostCompStage() override = default;

    protected:
        void setup(cref<sptr<engine::gfx::Device>> device_) override;

        void destroy() override;
    };
}

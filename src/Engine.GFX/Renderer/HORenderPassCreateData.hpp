#pragma once
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::gfx {
    class Texture;
}

namespace hg::engine::gfx::scene {
    class SceneView;
}

namespace hg::engine::scene {
    class IRenderScene;
}

namespace hg::engine::gfx::render {
    struct HORenderPassCreateData {
        sptr<Texture> target;

        ptr<::hg::engine::scene::IRenderScene> scene = nullptr;
        ptr<scene::SceneView> sceneView = nullptr;
    };
}

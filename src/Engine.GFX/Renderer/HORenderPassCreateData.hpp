#pragma once
#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::gfx {
    class Texture;
}

namespace ember::engine::gfx::scene {
    class SceneView;
}

namespace ember::engine::scene {
    class IRenderScene;
}

namespace ember::engine::gfx::render {

    struct HORenderPassCreateData {

        sptr<Texture> target;

        ptr<::ember::engine::scene::IRenderScene> scene = nullptr;
        ptr<scene::SceneView> sceneView = nullptr;
    };
}

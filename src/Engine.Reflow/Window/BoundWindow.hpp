#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::gfx {
    class RenderTarget;
    class Surface;
}

namespace ember::engine::gfx::scene {
    class SceneView;
}

namespace ember::engine::reflow {
    class Window;
}

namespace ember::engine::reflow {
    struct BoundWindow {
        sptr<gfx::RenderTarget> renderTarget;
        uptr<gfx::scene::SceneView> sceneView;
        non_owning_rptr<gfx::Surface> surface;
        sptr<Window> window;

        /**/

        u64 resizeListen;
    };
}

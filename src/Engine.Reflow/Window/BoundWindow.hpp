#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

namespace hg::engine::gfx {
    class RenderTarget;
    class Surface;
}

namespace hg::engine::gfx::scene {
    class SceneView;
}

namespace hg::engine::reflow {
    class Window;
}

namespace hg::engine::reflow {
    struct BoundWindow {
        sptr<gfx::RenderTarget> renderTarget;
        smr<const gfx::scene::SceneView> sceneView;
        non_owning_rptr<gfx::Surface> surface;
        sptr<Window> window;

        /**/

        u64 resizeListen;
    };
}

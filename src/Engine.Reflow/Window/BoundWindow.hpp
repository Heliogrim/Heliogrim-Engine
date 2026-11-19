#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    class RenderTarget;
    class Surface;
}

namespace hg::engine::reflow {
    class Window;
}

namespace hg::engine::reflow {
    struct BoundWindow {
        smr<graphics::RenderTarget> renderTarget;
        nmpt<graphics::Surface> surface;
        sptr<Window> window;

        /**/

        u64 resizeListen;
    };
}

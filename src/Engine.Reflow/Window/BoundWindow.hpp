#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

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

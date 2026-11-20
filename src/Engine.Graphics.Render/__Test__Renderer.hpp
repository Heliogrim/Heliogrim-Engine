#pragma once

#include "Renderer.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render {
    class TestRenderer :
        public Renderer {
    public:
        using this_type = TestRenderer;

    public:
        TestRenderer(
            mref<nmpt<graphics::cache::GlobalCacheCtrl>> globalCache_,
            mref<nmpt<graphics::memory::GlobalPooledAllocator>> globalGraphicsAllocator_
        ) noexcept;

        ~TestRenderer() noexcept override;

    private:
        [[nodiscard]] static uptr<graph::CompileGraph> makeCompileGraph() noexcept;
    };
}

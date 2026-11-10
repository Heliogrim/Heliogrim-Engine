#pragma once
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Renderer.hpp"

namespace hg::engine::render {
    class TestRenderer :
        public Renderer {
    public:
        using this_type = TestRenderer;

    public:
        TestRenderer(
            mref<nmpt<gfx::cache::GlobalCacheCtrl>> globalCache_,
            mref<nmpt<gfx::memory::GlobalPooledAllocator>> globalGfxAllocator_
        ) noexcept;

        ~TestRenderer() noexcept override;

    private:
        [[nodiscard]] static uptr<graph::CompileGraph> makeCompileGraph() noexcept;
    };
}

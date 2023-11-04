#pragma once
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Renderer.hpp"

namespace hg::engine::gfx::render {
    class TestRenderer :
        public Renderer {
    public:
        using this_type = TestRenderer;

    public:
        TestRenderer(
            mref<nmpt<cache::GlobalCacheCtrl>> globalCache_,
            mref<nmpt<memory::GlobalPooledAllocator>> globalGfxAllocator_
        ) noexcept;

        ~TestRenderer() noexcept override;

    private:
        [[nodiscard]] static uptr<graph::CompileGraph> makeCompileGraph() noexcept;
    };
}

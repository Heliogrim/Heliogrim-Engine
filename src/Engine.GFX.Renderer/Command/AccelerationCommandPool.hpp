#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "__fwd.hpp"

namespace hg::engine::gfx::render {
    class AccelerationCommandPool {
    public:
        [[nodiscard]] nmpt<AccelerationCommandBuffer> acquire();

        void release(mref<nmpt<AccelerationCommandBuffer>> buffer_);
    };
}

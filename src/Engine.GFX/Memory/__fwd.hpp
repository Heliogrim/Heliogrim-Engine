#pragma once

#include <Engine.Common/Types.hpp>

namespace ember::engine::gfx::memory {
    /**
     * Memory
     */
    struct AllocatedMemory;
    struct MemoryLayout;

    /**
     * Caching / Pooling
     */
    class MemoryCache;
    class MemoryPool;

    /**
     * Native (Layer) Allocation
     */
    class Allocator;
    class VkAllocator;

    /**
     * Hierarchy Allocators
     */
    class GlobalPooledAllocator;
    class LocalPooledAllocator;

    /**
     * Report
     */
    enum AllocationResult : u8;
}

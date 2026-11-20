#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics::memory {
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
     * (Layer) Allocation
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

#pragma once

#include "__fwd.hpp"
#include "MemoryLayout.hpp"

#include "MemoryMapping.hpp"
#include "../vkinc.hpp"

namespace ember::engine::gfx::memory {

    struct AllocatedMemory {

        ~AllocatedMemory();

        //
        ref<AllocatedMemory> operator=(cref<AllocatedMemory>) = delete;

        ref<AllocatedMemory> operator=(mref<AllocatedMemory> other_) noexcept;

        //
        ptr<Allocator> allocator;
        ptr<AllocatedMemory> parent;

        //
        MemoryLayout layout;

        u64 size;
        u64 offset;

        //
        vk::Device vkDevice;
        vk::DeviceMemory vkMemory;

        //
        MemoryMapping mapping;

        MemoryMapping map(const u64 size_, const u64 offset_ = 0ui64);

        void unmap();

        //
        void flush(const u64 size_, const u64 offset_ = 0ui64);

        bool write(const ptr<const void> data_, const u64 size_);

        //
        static void free(mref<ptr<AllocatedMemory>> memory_);
    };
}

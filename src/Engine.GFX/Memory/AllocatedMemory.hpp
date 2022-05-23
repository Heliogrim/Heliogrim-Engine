#pragma once

#include "__fwd.hpp"
#include "MemoryLayout.hpp"

#include "MemoryMapping.hpp"
#include "../Device/Device.hpp"

namespace ember::engine::gfx::memory {

    struct AllocatedMemory {

        ~AllocatedMemory();

        //
        ref<AllocatedMemory> operator=(cref<AllocatedMemory>) = delete;

        ref<AllocatedMemory> operator=(mref<AllocatedMemory> other_) noexcept;

        //
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

        MemoryMapping map(const u64 size_, const u64 offset_);

        void unmap();

        //
        void flush(const u64 size_, const u64 offset_);

        bool write(const ptr<const void> data_, const u64 size_);
    };
}

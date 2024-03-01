#pragma once

#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "__fwd.hpp"
#include "MemoryLayout.hpp"

#include "MemoryMapping.hpp"
#include "../vkinc.hpp"

namespace hg::engine::gfx::memory {
    struct AllocatedMemory {
        AllocatedMemory(
            nmpt<Allocator> allocator_,
            nmpt<AllocatedMemory> parent_,
            cref<MemoryLayout> layout_,
            cref<u64> size_,
            cref<u64> offset_,
            cref<vk::Device> vkDevice_,
            cref<vk::DeviceMemory> vkMemory_,
            MemoryMapping mapping_ = nullptr
        ) noexcept;

        AllocatedMemory(mref<AllocatedMemory> other_) noexcept;

        AllocatedMemory(cref<AllocatedMemory> other_) = delete;

        ~AllocatedMemory();

        //
        ref<AllocatedMemory> operator=(cref<AllocatedMemory>) = delete;

        ref<AllocatedMemory> operator=(mref<AllocatedMemory> other_) noexcept;

        //
        nmpt<Allocator> allocator;
        nmpt<AllocatedMemory> parent;

        //
        MemoryLayout layout;

        u64 size;
        u64 offset;

        //
        vk::Device vkDevice;
        vk::DeviceMemory vkMemory;

        //
        MemoryMapping mapping;

        MemoryMapping map(const u64 size_, const u64 offset_ = 0uLL);

        void unmap();

        //
        void flush(const u64 size_, const u64 offset_ = 0uLL);

        bool write(const ptr<const void> data_, const u64 size_);

        //
        static void free(mref<uptr<AllocatedMemory>> memory_);
    };
}

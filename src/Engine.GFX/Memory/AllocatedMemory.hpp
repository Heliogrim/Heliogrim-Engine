#pragma once
#include <Engine.Common/Types.hpp>

#include "MemoryMapping.hpp"
#include "MemoryProperty.hpp"
#include "../Device/Device.hpp"

namespace ember::engine::gfx {

    struct AllocatedMemory {

        ~AllocatedMemory();

        ref<AllocatedMemory> operator=(cref<AllocatedMemory> other_) = delete;

        ref<AllocatedMemory> operator=(mref<AllocatedMemory> other_) noexcept;

        MemoryProperties props;

        u64 align;
        u64 size;

        MemoryMapping mapping;

        vk::Device vkDevice;
        vk::DeviceMemory vkMemory;

        void flush(const u64 size_, const u64 offset_);

        MemoryMapping map(const u64 size_, const u64 offset_);

        void unmap();

        bool write(const ptr<void> data_, const u64 size_);

    };

}

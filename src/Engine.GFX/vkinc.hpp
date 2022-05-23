#pragma once

#include <Engine.Common/stdafx.h>

namespace ember::engine::gfx {
    // TODO: Refactor / Move to other location
    inline uint32_t get_memory_type(const vk::PhysicalDevice& device_, const uint32_t types_,
        const vk::MemoryPropertyFlags& flags_) {
        const vk::PhysicalDeviceMemoryProperties memProps = device_.getMemoryProperties();

        for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
            if ((types_ & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & flags_) == flags_)
                return i;
        }

        throw std::runtime_error("Failed to find suitable memory type.");
    }
}

#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Math/Matrix.hpp>

/**
 * Graphic includes
 */
#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

// #define VULKAN_HPP_DISABLE_ENHANCED_MODE
// #define VULKAN_HPP_NO_CONSTRUCTORS
// #define VULKAN_HPP_NO_SETTERS
// #define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_NO_SPACESHIP_OPERATOR

START_SUPPRESS_WARNINGS
#include <vulkan/vulkan.hpp>
STOP_SUPPRESS_WARNINGS

namespace hg::engine::gfx {
    // TODO: Refactor / Move to other location
    inline uint32_t get_memory_type(
        const vk::PhysicalDevice& device_,
        const uint32_t types_,
        const vk::MemoryPropertyFlags& flags_
    ) {
        const vk::PhysicalDeviceMemoryProperties memProps = device_.getMemoryProperties();

        for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
            if ((types_ & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & flags_) == flags_)
                return i;
        }

        throw std::runtime_error("Failed to find suitable memory type.");
    }

    /**
     * Matrix used to "normalize" vectors like "position" or "normal" (only when applied to matrix-vector chain)
     */
    constexpr static inline math::mat4 vk_norm_mat_v {
        1.0, 0.0, 0.0, 0.0,
        0.0, -1.0, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.5,
        0.0, 0.0, 0.0, 1.0
    };

    /**
     * Matrix used to "normalize" matrices like "model matrix" (only when used with matrix chain or to inverse corrected projection)
     */
    constexpr static inline math::mat4 vk_norm_mat_m {
        1.0, 0.0, 0.0, 0.0,
        0.0, -1.0, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.0, 0.0, 0.5, 1.0
    };
}

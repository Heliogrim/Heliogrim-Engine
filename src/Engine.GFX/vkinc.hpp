#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Math/Matrix.hpp>

/**
 * Graphic includes
 */
#if defined(VK_IMPL_PLATFORM)

#if not(defined(VK_USE_PLATFORM_ANDROID_KHR) || defined(VK_USE_PLATFORM_XLIB_KHR) || defined(VK_USE_PLATFORM_WAYLAND_KHR) || defined(VK_USE_PLATFORM_WIN32_KHR))
#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__)
// TODO: Check how we want to deal with X11 vs Wayland
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#endif

#else

#undef VK_USE_PLATFORM_WIN32_KHR
#undef VK_USE_PLATFORM_XLIB_KHR
#undef VK_USE_PLATFORM_WAYLAND_KHR
#undef VK_USE_PLATFORM_ANDROID_KHR
#undef VK_USE_PLATFORM_XCB_KHR
#endif

// #define VULKAN_HPP_DISABLE_ENHANCED_MODE
// #define VULKAN_HPP_NO_CONSTRUCTORS
// #define VULKAN_HPP_NO_SETTERS
// #define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_NO_SPACESHIP_OPERATOR

#ifndef VK_NULL_HANDLE
#define VK_NULL_HANDLE nullptr
#endif

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
}

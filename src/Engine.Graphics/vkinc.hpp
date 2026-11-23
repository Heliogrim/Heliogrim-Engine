#pragma once

#include <Engine.Common/__macro.hpp>

// Included first, so the real headers below verify every forward declaration
#include "API/__vkFwd.hpp"

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
// Compile-time trim: vk::to_string and Unique/Shared handles are unused
#define VULKAN_HPP_NO_TO_STRING
#define VULKAN_HPP_NO_SMART_HANDLE

#ifndef VK_NULL_HANDLE
#define VK_NULL_HANDLE nullptr
#endif

#if defined(VK_IMPL_PLATFORM)
/* The platform TU follows `import std;` (from __vkFwd.hpp); vulkan.hpp's module mode imports std instead of including std headers. */
#define VULKAN_HPP_CXX_MODULE 1
START_SUPPRESS_WARNINGS
#include <vulkan/vulkan_hpp_macros.hpp>
#include <vulkan/vulkan.hpp>
STOP_SUPPRESS_WARNINGS
#elif defined(HG_MODULE_INTERFACE_UNIT)
// Module global fragments get C-level declarations only; the vulkan module lists vulkan.hpp in its own prelude.
#include <assert.h>
#include <string.h>
#include <vulkan/vulkan_hpp_macros.hpp>
#include <vulkan/vulkan_core.h>
#else
// As vulkan.hpp's own module mode: macros and C functions do not come through the import.
#include <cassert>
#include <cstring>
#include <vulkan/vulkan_hpp_macros.hpp>
#include <vulkan/vulkan_core.h>
import vulkan;
#endif


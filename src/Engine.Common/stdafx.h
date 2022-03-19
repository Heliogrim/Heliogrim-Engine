#pragma once

#include "__macro.hpp"

#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <stddef.h>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#ifdef _DEBUG
#include <assert.h>
#endif

#if _WIN32 || _WIN64
#include <windows.h>
#include <windowsx.h>
#endif

#include "stddg.h"

#include "Exception/__macro.hpp"
#include "Make.hpp"

// Math
#include <cmath>

using size64_t = uint64_t;

// Memory
#define SAFE_DELETE(var_) delete var_; var_ = nullptr;
#define SAFE_DELETE_ARR(var_) delete[] var_; var_ = nullptr;

// Time
#include <ctime>

/**
 * Error Includes
 */
#include "./Exception/NotImplementedException.hpp"
/**
 *
 */

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

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/trigonometric.hpp>

#include "./Math/Matrix.hpp"
#include "./Math/Vector.hpp"

// #define VULKAN_HPP_DISABLE_ENHANCED_MODE
// #define VULKAN_HPP_NO_CONSTRUCTORS
// #define VULKAN_HPP_NO_SETTERS
// #define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_NO_SPACESHIP_OPERATOR
#include <vulkan/vulkan.hpp>
/**
 *
 */

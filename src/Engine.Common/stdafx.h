#pragma once

#include "__macro.hpp"

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <stddef.h>
#include <type_traits>

#include <cassert>

#if _WIN32 || _WIN64
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>
#endif

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
 * Graphic includes
 */
#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif
/**
 *
 */

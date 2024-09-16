#pragma once

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#if !defined(__FUNCSIG__)
#define __FUNCSIG__ __PRETTY_FUNCTION__
#endif

// Windows Environment Check
#if defined(_MSC_VER) || _WIN32 || _WIN64

#define ENV_MSVC TRUE
#define ENV_WIN TRUE

#if defined(_M_AMD64)
#define ENVx64 TRUE
#else
#define ENVx32 TRUE
#endif

// Check GCC
#elif defined(__GNUC__)

#define ENV_GCC TRUE

#if defined(__x86_64__)
#define ENVx64 TRUE
#else
#define ENVx32 TRUE
#endif

// Check Clang
#elif defined(__clang__)

#define ENV_CLANG TRUE

#if defined(__x86_64__)
#define ENVx64 TRUE
#elif defined(__i386)
#define ENVx32 TRUE
#elif defined(__aarch64__)
#define ENV_ARM TRUE
#define ENVx64 TRUE
#elif defined(__arm__)
#define ENV_ARM TRUE
#define ENVx32 TRUE
#endif

#else
#error "Compiler not supported."
#endif

#if defined(ENVx64) || defined(ENVx32)
#define CACHE_LINE_SIZE 64
#define DIST_CACHE_LINE_SIZE 64
#endif

// fixes compiler error cause by extended aligned storage
#define _ENABLE_EXTENDED_ALIGNED_STORAGE 1

#ifdef __GNUC__
#define DEPRECATED(fnc) fnc __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED(fnc) __declspec(deprecated) fnc
#elif __clang__
#define DEPRECTATED(fnc) __declspec(deprecated) fnc
#else
#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#define DEPRECATED(fnc) fnc
#endif

#define CONCATENATE(var0_,var1_) var0_ ## var1_
#define DOUBLE(var0_,var1_) CONCATENATE(var0_,var1_)
#define STRINGIZE(var_) #var_
#define QUOTE(var_) STRINGIZE(var_)

#define FLOAT_T(var_) static_cast<float_t>(var_)
#define INT8_T(var_) static_cast<int8_t>(var_)
#define UINT8_T(var_) static_cast<uint8_t>(var_)
#define INT32_T(var_) static_cast<int32_t>(var_)
#define UINT32_T(var_) static_cast<uint32_t>(var_)

// Warnings
#include "__warning.hpp"

// Alignment
#if defined(ENV_MSVC)
#define ALIGNED(struct_, alignment_) __declspec(align(alignment_)) struct_
#elif defined(ENV_GCC) || defined(ENV_CLANG)
#define ALIGNED(struct_, alignment_) __attribute__ ((aligned (alignment_))) struct_
#else
// TODO: Deal with error case
#defined ALIGNED(struct_, alignment_) struct_
#endif

// Comparison
#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

// Inline
#ifndef FORCE_INLINE
#if defined (ENV_MSVC)
#define FORCE_INLINE __forceinline
#elif defined(ENV_GCC) || defined(ENV_CLANG)
#define FORCE_INLINE __attribute__((always_inline))
#else
#define FORCE_INLINE inline
#endif
#endif

// Restrict
#ifndef RESTRICT
#if defined(ENV_MSVC) && not defined(RESTRICT)
#define RESTRICT __restrict
#endif

#if defined(ENV_GCC) && not defined(RESTRICT)
#define RESTRICT __restrict__
#endif

#if defined(ENV_CLANG) && not defined(RESTRICT)
#define RESTRICT __restrict__
#endif

#if not defined(RESTRICT)
#define RESTRICT
#endif
#endif

#if not defined(ENV_MSVC) && not defined(__restrict)
#define __restrict RESTRICT
#endif

// Attributes
#if not defined(support_no_unique_address)
#if __has_cpp_attribute(no_unique_address)
#define support_no_unique_address no_unique_address
#endif

#if not defined(support_no_unique_address) && ENV_MSVC && __has_cpp_attribute(msvc::no_unique_address)
#define support_no_unique_address msvc::no_unique_address
#endif

#if not defined(support_no_unique_address)
#define support_no_unique_address
#endif
#endif

#if not defined(macro_novtable)
#if defined(ENV_MSVC)
#define macro_novtable __declspec(novtable)
#else
#define macro_novtable
#endif
#endif

#if not defined(macro_attr_intr)
#if defined(ENV_MSVC) && not defined(__clang__)
#define macro_attr_intr [[msvc::intrinsic]]
#else
#define macro_attr_intr
#endif
#endif

// Profiling
#ifdef _PROFILING
//#define SCOPED_STOPWATCH_V(var_) hg::profiling::ScopedStopwatch var_ {__FUNCSIG__};
//#define SCOPED_STOPWATCH SCOPED_STOPWATCH_V(DOUBLE(__stopwatch__, __LINE__))
#define SCOPED_STOPWATCH_V(var_)
#define SCOPED_STOPWATCH
#else
#define SCOPED_STOPWATCH_V(var_)
#define SCOPED_STOPWATCH
#endif

#if defined(_DEBUG) || defined(_PROFILING)
#define _EDITOR
#endif

#define _EDITOR

#if defined (_DEBUG) || defined(_PROFILING)
#define IM_CORE_LOG(msg_) ::hg::Logger::info(msg_)
#define IM_CORE_INFO(msg_) ::hg::Logger::info(msg_)
#define IM_CORE_WARN(msg_) ::hg::Logger::warn(msg_)
#define IM_CORE_ERROR(msg_) ::hg::Logger::error(msg_)
#define IM_DEBUG_LOG(msg_) ::hg::Logger::debug(msg_)

START_SUPPRESS_WARNINGS
#define IM_CORE_LOGF(format_, ...) ::hg::Logger::info(format_, ##__VA_ARGS__)
#define IM_CORE_INFOF(format_, ...) ::hg::Logger::info(format_, ##__VA_ARGS__)
#define IM_CORE_WARNF(format_, ...) ::hg::Logger::warn(format_, ##__VA_ARGS__)
#define IM_CORE_ERRORF(format_, ...) ::hg::Logger::error(format_, ##__VA_ARGS__)
#define IM_DEBUG_LOGF(format_, ...) ::hg::Logger::debug(format_, ##__VA_ARGS__)
STOP_SUPPRESS_WARNINGS
#else
#define IM_CORE_LOG(msg_) ::hg::Logger::info(msg_)
#define IM_CORE_INFO(msg_) ::hg::Logger::info(msg_)
#define IM_CORE_WARN(msg_) ::hg::Logger::warn(msg_)
#define IM_CORE_ERROR(msg_) ::hg::Logger::error(msg_)
#define IM_DEBUG_LOG(msg_)

START_SUPPRESS_WARNINGS
#define IM_CORE_LOGF(format_, ...) ::hg::Logger::info(format_, ##__VA_ARGS__)
#define IM_CORE_INFOF(format_, ...) ::hg::Logger::info(format_, ##__VA_ARGS__)
#define IM_CORE_WARNF(format_, ...) ::hg::Logger::warn(format_, ##__VA_ARGS__)
#define IM_CORE_ERRORF(format_, ...) ::hg::Logger::error(format_, ##__VA_ARGS__)
#define IM_DEBUG_LOGF(format_, ...)
STOP_SUPPRESS_WARNINGS
#endif

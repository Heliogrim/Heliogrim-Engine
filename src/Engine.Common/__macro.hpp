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

#if !defined(__FUNCSIG__) && defined(__PRETTY_FUNCTION__)
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
#elif defined(__GNUG__)

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
#if __clang__
#define START_SUPPRESS_WARNINGS \
    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Wall\"") \
    _Pragma("clang diagnostic ignored \"-Weverything\"")
#define STOP_SUPPRESS_WARNINGS \
    _Pragma("clang diagnostic pop")
#elif defined(_MSC_VER)
#define START_SUPPRESS_WARNINGS \
    __pragma(warning(push))
#define STOP_SUPPRESS_WARNINGS \
    __pragma(warning(pop))
#else
#define START_SUPPRESS_WARNINGS
#define STOP_SUPPRESS_WARNINGS
#endif

// Alignment
#ifdef __GNUC__
#define ALIGNED(struct_, alignment_) struct_ __attribute__ ((aligned (alignment_)))
#else
#define ALIGNED(struct_, alignment_) __declspec(align(alignment_)) struct_
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

// Function Helper

#if not defined(Fn)
#define Fn(fn_) [](auto&&... fnArgs_) { return fn_(std::forward<decltype(fnArgs_)>(fnArgs_)...); }
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

#define IM_CORE_LOGF(format_, ...) ::hg::Logger::info(format_, ##__VA_ARGS__)
#define IM_CORE_INFOF(format_, ...) ::hg::Logger::info(format_, ##__VA_ARGS__)
#define IM_CORE_WARNF(format_, ...) ::hg::Logger::warn(format_, ##__VA_ARGS__)
#define IM_CORE_ERRORF(format_, ...) ::hg::Logger::error(format_, ##__VA_ARGS__)
#define IM_DEBUG_LOGF(format_, ...) ::hg::Logger::debug(format_, ##__VA_ARGS__)
#else
#define IM_CORE_LOG(msg_) ::hg::Logger::info(msg_)
#define IM_CORE_INFO(msg_) ::hg::Logger::info(msg_)
#define IM_CORE_WARN(msg_) ::hg::Logger::warn(msg_)
#define IM_CORE_ERROR(msg_) ::hg::Logger::error(msg_)
#define IM_DEBUG_LOG(msg_)

#define IM_CORE_LOGF(format_, ...) ::hg::Logger::info(format_, ##__VA_ARGS__)
#define IM_CORE_INFOF(format_, ...) ::hg::Logger::info(format_, ##__VA_ARGS__)
#define IM_CORE_WARNF(format_, ...) ::hg::Logger::warn(format_, ##__VA_ARGS__)
#define IM_CORE_ERRORF(format_, ...) ::hg::Logger::error(format_, ##__VA_ARGS__)
#define IM_DEBUG_LOGF(format_, ...)
#endif

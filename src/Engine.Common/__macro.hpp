#pragma once

#ifndef _STD
#define _STD ::std::
#endif

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

// Windows Environment Check
#if defined(_MSC_VER) || _WIN32 || _WIN64

#define ENV_MSVC TRUE

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

#define ASSERT(var_) if (!(var_)) { throw std::exception("Assertation failed."); }
#ifdef _DEBUG
#define DEBUG_ASSERT(var_, msg_) assert(var_ && msg_);
#define DEBUG_STATIC_ASSERT(var_, msg_) static_assert(var_, msg_);
#else
#define DEBUG_ASSERT(var_, msg_)
#define DEBUG_STATIC_ASSERT(var_, msg_)
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
#define FORCE_INLINE __forceinline
#endif

// Restrict
#ifndef RESTRICT
#if ENV_MSVC && not defined(RESTRICT)
#define RESTRICT __restrict
#endif

#if ENV_GCC && not defined(RESTRICT)
#define RESTRICT __restrict__
#endif

#if ENV_CLANG && not defined(RESTRICT)
#endif

#if not defined(RESTRICT)
#define restrict
#endif
#endif

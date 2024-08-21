#pragma once

/* Warning: General tool which will suppress all warnings...*/

#if __clang__
#define START_SUPPRESS_WARNINGS \
_Pragma("clang diagnostic push") \
_Pragma("clang diagnostic ignored \"-Wall\"") \
_Pragma("clang diagnostic ignored \"-Weverything\"")
#define STOP_SUPPRESS_WARNINGS \
_Pragma("clang diagnostic pop")
#elif defined(_MSC_VER)
#define START_SUPPRESS_WARNINGS \
__pragma(warning(push, 0))
#define STOP_SUPPRESS_WARNINGS \
__pragma(warning(pop))
#elif __GNUC__
#define START_SUPPRESS_WARNINGS \
_Pragma("GCC diagnostic push") \
_Pragma("GCC diagnostic ignored \"-Wall\"") \
_Pragma("GCC diagnostic ignored \"-Wdelete-incomplete\"")
#define STOP_SUPPRESS_WARNINGS \
_Pragma("GCC diagnostic pop")
#else
#define START_SUPPRESS_WARNINGS
#define STOP_SUPPRESS_WARNINGS
#endif

/* Purposely bound warning helpers... */

#if __clang__
#define BEGIN_REGION_UNCHECKED_LLVL \
_Pragma("clang diagnostic push") \
_Pragma("clang diagnostic ignored \"-Wunsafe-buffer-usage\"")
#define END_REGION_UNCHECKED_LLVL \
_Pragma("clang diagnostic pop")
#elif defined(_MSC_VER)
#define BEGIN_REGION_UNCHECKED_LLVL \
__pragma(warning(push)) \
__pragma(warning(disable: 26481))
#define END_REGION_UNCHECKED_LLVL \
__pragma(warning(pop))
#elif __GNUC__
// GCC does not allow pragma inside functions, don't care anymore at this point
#define BEGIN_REGION_UNCHECKED_LLVL
#define END_REGION_UNCHECKED_LLVL
#else
#define BEGIN_REGION_UNCHECKED_LLVL
#define END_REGION_UNCHECKED_LLVL
#endif

#define GUARD_UNCHECKED_LLVL(expr) \
BEGIN_REGION_UNCHECKED_LLVL \
expr \
END_REGION_UNCHECKED_LLVL

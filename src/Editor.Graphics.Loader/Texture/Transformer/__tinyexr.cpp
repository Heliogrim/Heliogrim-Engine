#include <Engine.Common/__macro.hpp>

#define TINYEXR_USE_MINIZ FALSE
#define TINYEXR_USE_STB_ZLIB FALSE
#define TINYEXR_USE_NANOZLIB FALSE
#define TINYEXR_USE_THREAD FALSE
#define TINYEXR_USE_OPENMP FALSE

#define TINYEXR_IMPLEMENTATION
#include <zlib/zlib.h>
#include <tinyexr/tinyexr.h>

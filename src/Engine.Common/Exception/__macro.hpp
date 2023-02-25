#pragma once

#include "../__macro.hpp"

#define EXCEPTION_UNSUPPORTED_METHOD throw _STD runtime_error("Unsupported method exception");
#define EXCEPTION_UNSUPPORTED_CONSTRUCTOR throw _STD runtime_error("Unsupported constructor exception");

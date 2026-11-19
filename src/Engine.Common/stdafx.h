#pragma once

#include <cassert>
#include <cstdint>

#include "__macro.hpp"

#if _WIN32 || _WIN64
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef STRICT
#define STRICT
#endif

#include <Windows.h>
#include <windowsx.h>
#endif

// Math
#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import std;
#endif

using size64_t = uint64_t;

// Memory
#define SAFE_DELETE(var_) delete var_; var_ = nullptr;
#define SAFE_DELETE_ARR(var_) delete[] var_; var_ = nullptr;

// Time
#include <ctime>

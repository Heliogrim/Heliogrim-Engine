//
// pch.h
//

#pragma once

#include "gtest/gtest.h"

#include <Engine.Common/Expected.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Variant.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Reflect/IsType.hpp>
#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Asserts;
#endif

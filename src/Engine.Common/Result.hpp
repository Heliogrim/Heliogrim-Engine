#pragma once

#include "Inc.hpp"

namespace hg::__internal::result {
    enum Result: uint8_t {
        eNone = 0x00000000,
        eUnknown = 0x00000001,
        eSuccess = 0x00000002
    };
}

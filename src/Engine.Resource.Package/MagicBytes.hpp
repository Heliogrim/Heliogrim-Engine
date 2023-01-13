#pragma once

#include <Engine.Common/Types.hpp>

namespace ember::engine::resource {
    constexpr static inline u8 PackageMagicBytes[6] = { 0x49, 0x4D, 0x50, 0x41, 0x43, 0x4B };
    constexpr static inline u8 PackageMagicVersion[1] = { 0x01 };
}

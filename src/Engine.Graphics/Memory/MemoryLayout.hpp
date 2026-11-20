#pragma once

#include "MemoryProperty.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics::memory {
    struct MemoryLayout final {
        u64 align;
        MemoryProperties props;
        u32 typeBits;
    };

    [[nodiscard]] inline bool operator==(cref<MemoryLayout> left_, cref<MemoryLayout> right_) noexcept {
        return left_.align == right_.align &&
            left_.props.unwrap == right_.props.unwrap &&
            left_.typeBits == right_.typeBits;
    }
}

namespace std {
    template <>
    struct hash<hg::engine::graphics::memory::MemoryLayout> {
        [[nodiscard]] size_t operator()(hg::cref<hg::engine::graphics::memory::MemoryLayout> obj_) const noexcept {
            return (hg::u64 { obj_.typeBits } << 32uLL) ^
                (obj_.align << 6uLL) ^
                static_cast<hg::u64>(obj_.props.unwrap);
        }
    };
}

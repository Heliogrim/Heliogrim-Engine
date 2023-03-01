#pragma once

#include <Engine.Common/Types.hpp>

#include "MemoryProperty.hpp"

namespace hg::engine::gfx::memory {
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
    struct hash<hg::engine::gfx::memory::MemoryLayout> {
        [[nodiscard]] size_t operator()(hg::cref<hg::engine::gfx::memory::MemoryLayout> obj_) const noexcept {
            return (hg::u64 { obj_.typeBits } << 32ui64) ^
                (obj_.align << 6ui64) ^
                static_cast<hg::u64>(obj_.props.unwrap);
        }
    };
}

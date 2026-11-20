#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    enum class MemoryProperty {
        eUndefined = 0x0,
        // 0b0000'0000
        //
        eDeviceLocal = 0x1,
        // 0b0000'0001
        eHostVisible = 0x2,
        // 0b0000'0010
        eHostCoherent = 0x4,
        // 0b0000'0100
        eHostCached = 0x8,
        // 0b0000'1000
        eLazilyAllocated = 0x10,
        // 0b0001'0000
        eProtected = 0x20// 0b0010'0000
    };

    typedef Flag<MemoryProperty, hg::u8> MemoryProperties;

    constexpr MemoryProperties operator|(cref<MemoryProperty> left_, cref<MemoryProperty> right_) noexcept {
        return MemoryProperties { left_ } |= right_;
    }
}

#pragma once

namespace hg::engine::accel {
    enum class SpirvTargetVersion {
        eSpv10 = (0x1 << 16),
        eSpv11 = (0x1 << 16) | (0x1 << 8),
        eSpv12 = (0x1 << 16) | (0x2 << 8),
        eSpv13 = (0x1 << 16) | (0x3 << 8),
        eSpv14 = (0x1 << 16) | (0x4 << 8),
        eSpv15 = (0x1 << 16) | (0x5 << 8),
        eSpv16 = (0x1 << 16) | (0x6 << 8)
    };
}

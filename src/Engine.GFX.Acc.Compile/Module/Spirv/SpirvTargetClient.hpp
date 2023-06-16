#pragma once

namespace hg::engine::gfx::acc {
    enum class SpirvTargetClient {
        eVulkan13 = (0x1 << 22) | (0x3 << 12)
    };
}

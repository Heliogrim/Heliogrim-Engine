#pragma once

namespace ember::engine::core {
    enum class WorldType {
        eUndefined = 0x0,
        eGame = 0x1,
        eEditor = 0x2,
        ePreview = 0x3
    };
}

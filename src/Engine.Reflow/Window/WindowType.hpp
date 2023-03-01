#pragma once

namespace hg::engine::reflow {
    enum class WindowType {
        // Windowed, Title Bar, Border
        eNormal,
        // Windowed, No Title Bar, No Border
        eBorderless,
        // Fullscreen, No Title Bar, No Border
        eFullscreen
    };
}

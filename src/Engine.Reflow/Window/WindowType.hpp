#pragma once

namespace ember::engine::reflow {

    enum class WindowType {
        eNormal, // Windowed, Title Bar, Border
        eBorderless, // Windowed, No Title Bar, No Border
        eFullscreen, // Fullscreen, No Title Bar, No Border
    };

}
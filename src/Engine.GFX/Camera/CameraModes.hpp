#pragma once

namespace ember::engine::gfx {
    enum class CameraProjectionMode {
        ePerspective = false,
        eOrthographic = true
    };

    enum class CameraViewMode {
        eFirstPerson = false,
        eLookAt = true
    };
}

#pragma once

namespace hg::engine::graphics {
    enum class CameraProjectionMode {
        ePerspective = false,
        eOrthographic = true
    };

    enum class CameraViewMode {
        eFirstPerson = false,
        eLookAt = true
    };
}

#pragma once

namespace hg::engine::graphics {
    enum class CameraProjectionMode : unsigned char {
        ePerspective = false,
        eOrthographic = true
    };

    enum class CameraViewMode : unsigned char {
        eFirstPerson = false,
        eLookAt = true
    };
}

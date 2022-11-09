#pragma once

#include <Engine.Common/Types.hpp>

namespace ember::engine::reflow {

    enum class TextAlign : u8 {
        eLeftTop = 0b0001'0001,
        eLeftMiddle = 0b0010'0001,
        eLeftBottom = 0b0100'0001,
        eCenterTop = 0b0001'0010,
        eCenterMiddle = 0b0010'0010,
        eCenterBottom = 0b0100'0010,
        eRightTop = 0b0001'0100,
        eRightMiddle = 0b0010'0100,
        eRightBottom = 0b0100'0100
    };

}

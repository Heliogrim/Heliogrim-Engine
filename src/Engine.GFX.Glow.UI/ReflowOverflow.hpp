#pragma once

#include <Engine.Common/Types.hpp>

namespace ember::engine::gfx::glow::ui {

    enum class ReflowOverflow : u8 {
        eNone = 0ui8,
        eAuto = 1ui8,
        eScroll = 2ui8,
        eDiscard = 3ui8
    };

}

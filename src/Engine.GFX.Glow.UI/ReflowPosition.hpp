#pragma once

#include <Engine.Common/Types.hpp>

namespace ember::engine::gfx::glow::ui {

    enum class ReflowPosition : u8 {
        eStatic = 0ui8,
        eRelative = 1ui8,
        eAbsolute = 2ui8
    };

}

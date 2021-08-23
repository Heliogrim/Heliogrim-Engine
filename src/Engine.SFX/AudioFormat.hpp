#pragma once
#include <Engine.Common/Flag.hpp>
#include <Engine.Common/Types.hpp>

namespace ember::engine::sfx {
    enum class AudioFormat {
        eUndefined = 0x0,
        //
        eS8 = 0x001,
        eS16 = 0x002,
        eS32 = 0x003,
        eU8 = 0x101,
        eU16 = 0x102,
        eU32 = 0x103,
        eF16 = 0x201,
        eF32 = 0x202
    };

    typedef Flag<AudioFormat, ember::u16> AudioFormats;
}

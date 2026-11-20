#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::audio {
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

    typedef Flag<AudioFormat, hg::u16> AudioFormats;
}

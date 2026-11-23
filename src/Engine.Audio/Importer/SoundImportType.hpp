#pragma once

#include <Engine.Common/Buffer.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets.Type;
import Engine.Common;
#endif

namespace hg::engine::audio {
    struct SoundImportType {
        /** Defines the amount of supplied channels */
        u8 channels = 0u;

        /** Describes the format the data is supplied with */
        AudioFormat format = AudioFormat::eUndefined;

        /** Defines the amount of supplied samples */
        u64 samples = 0uLL;

        /** Defines the amount of samples per second */
        u64 sampleRate = 0uLL;

        /** The buffer where the supplied data is stored */
        Buffer buffer;
    };
}

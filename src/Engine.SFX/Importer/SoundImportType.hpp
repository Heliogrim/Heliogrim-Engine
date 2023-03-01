#pragma once
#include <Engine.Common/Buffer.hpp>

#include "../AudioFormat.hpp"

namespace hg::engine::sfx {
    struct SoundImportType {
        /** Defines the amount of supplied channels */
        u8 channels = 0ui8;

        /** Describes the format the data is supplied with */
        AudioFormat format = AudioFormat::eUndefined;

        /** Defines the amount of supplied samples */
        u64 samples = 0ui64;

        /** Defines the amount of samples per second */
        u64 sampleRate = 0ui64;

        /** The buffer where the supplied data is stored */
        Buffer buffer;
    };
}

#pragma once

#include "__fwd.hpp"

namespace ember::engine::gfx::loader {
    enum class TextureLoadDataFlagBits : u8 {
        eNone = 0b0000 << 0,
        eLazyDataLoading = 0b0001 << 0,
        eLockLoaded = 0b0001 << 1,
    };

    using TextureLoadDataFlags = Flag<TextureLoadDataFlagBits>;

    struct TextureLoadOptions {
        TextureLoadDataFlags dataFlag;
    };
}

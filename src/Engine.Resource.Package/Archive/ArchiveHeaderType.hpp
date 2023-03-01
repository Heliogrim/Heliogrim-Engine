#pragma once

#include <Engine.Common/Types.hpp>

#include "__fwd.hpp"

namespace hg::engine::resource {
    enum class ArchiveHeaderType : u8 {
        eUndefined = 0x0,
        //
        eSerializedStructure = 0x2,
        eSerializedLayout = 0x3,
        //
        eRaw = 0xFF,
    };
}

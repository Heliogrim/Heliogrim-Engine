#pragma once

#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Types.hpp>

#include "__fwd.hpp"
#include "ArchiveHeaderType.hpp"

namespace hg::engine::resource {
    #pragma pack(push)
    #pragma pack(1)
    struct ArchiveHeader {
        ArchiveHeaderType type;
        Guid guid;
    };
    #pragma pack(pop)
}

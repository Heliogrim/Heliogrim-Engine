#pragma once

#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Types.hpp>

#include "__fwd.hpp"
#include "ArchiveHeaderType.hpp"

namespace ember::engine::resource {
    struct ArchiveHeader {
        Guid guid;
        ArchiveHeaderType type;
    };
}

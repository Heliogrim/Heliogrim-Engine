#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Serialization/Archive/__fwd.hpp>

#include "../Archive/ArchiveHeader.hpp"
#include "../Package/PackageIndexEntry.hpp"

namespace hg::engine::resource {
    struct LinkedArchive {
        ArchiveHeader header;
        PackageIndexEntry index;
    };
}

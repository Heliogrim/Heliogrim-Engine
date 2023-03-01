#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"

namespace hg::engine::resource {
    struct PackageIndex {
        ptr<PackageIndexEntry> entries;
    };
}

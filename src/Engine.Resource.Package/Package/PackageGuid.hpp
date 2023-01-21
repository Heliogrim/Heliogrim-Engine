#pragma once

#include <Engine.Common/Guid.hpp>

namespace ember::engine::resource {
    class PackageGuid final :
        public Guid {
    public:
        using this_type = PackageGuid;

    public:
        using Guid::Guid;
    };
}

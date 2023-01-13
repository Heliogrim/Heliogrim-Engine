#pragma once

#include "__fwd.hpp"

#include "PackageHeader.hpp"
#include "PackageFooter.hpp"

namespace ember::engine::resource {
    class Package {
    private:
        PackageHeader _header;
        PackageFooter _footer;
    };
}

#pragma once

#include "__fwd.hpp"
#include "Linker/__fwd.hpp"

#include "PackageHeader.hpp"
#include "PackageFooter.hpp"

namespace ember::engine::resource {
    class Package {
    private:
        PackageHeader _header;
        PackageFooter _footer;

    private:
        uptr<PackageLinker> _linker;

    public:
        [[nodiscard]] const non_owning_rptr<PackageLinker> getLinker() const noexcept;
    };
}

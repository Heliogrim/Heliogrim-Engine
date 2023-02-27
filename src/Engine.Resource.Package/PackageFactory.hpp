#pragma once

#include "Package/Package.hpp"

namespace hg::engine::resource {
    class PackageFactory final {
    public:
        using this_type = PackageFactory;

    public:
        [[nodiscard]] static uptr<Package> createEmptyPackage(mref<uptr<Source>> source_);

        [[nodiscard]] static uptr<Package> createFromSource(mref<uptr<Source>> source_);
    };
}

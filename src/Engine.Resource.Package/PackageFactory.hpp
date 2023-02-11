#pragma once

#include "Package/Package.hpp"

namespace ember::engine::resource {
    class PackageFactory final {
    public:
        using this_type = PackageFactory;

    public:
        [[nodiscard]] static Package createEmptyPackage(mref<uptr<Source>> source_);

        [[nodiscard]] static Package createFromSource(mref<uptr<Source>> source_);
    };
}

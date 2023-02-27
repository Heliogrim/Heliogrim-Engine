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

namespace std {
    template <>
    struct hash<::ember::engine::resource::PackageGuid> :
        _STD hash<::ember::Guid> {
        [[nodiscard]] size_t operator()(::ember::cref<::ember::engine::resource::PackageGuid> value_) const noexcept {
            return static_cast<::ember::cref<_STD hash<::ember::Guid>>>(*this)(value_);
        }
    };
}

#pragma once

#include <Engine.Common/Guid.hpp>

namespace hg::engine::resource {
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
    struct hash<::hg::engine::resource::PackageGuid> :
        std::hash<::hg::Guid> {
        [[nodiscard]] size_t operator()(::hg::cref<::hg::engine::resource::PackageGuid> value_) const noexcept {
            return static_cast<::hg::cref<std::hash<::hg::Guid>>>(*this)(value_);
        }
    };
}

#pragma once

namespace hg {
    struct nothrow_t {
        constexpr explicit nothrow_t() = default;
    };

    namespace traits {
        inline constexpr nothrow_t nothrow;
    };
}

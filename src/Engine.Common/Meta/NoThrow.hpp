#pragma once

namespace hg {
    struct nothrow_t {
        constexpr explicit nothrow_t() = default;
    };

    namespace traits {
        constexpr static nothrow_t nothrow;
    };
}

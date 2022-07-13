#pragma once

namespace ember {

    struct sync_t {
        constexpr explicit sync_t() = default;
    };

    namespace traits {
        extern sync_t const sync;
    }

    struct async_t {
        constexpr explicit async_t() = default;
    };

    namespace traits {
        extern async_t const async;
    }

}

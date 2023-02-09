#pragma once

namespace ember {
    struct async_t {
        constexpr explicit async_t() = default;
    };

    namespace traits {
        constexpr static async_t async;
    };

    struct sync_t {
        constexpr explicit sync_t() = default;
    };

    namespace traits {
        constexpr static sync_t sync;
    };
}

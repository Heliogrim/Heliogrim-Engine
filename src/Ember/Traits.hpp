#pragma once

namespace ember {

    struct sync_t {
        explicit sync_t() = default;
    };

    extern sync_t const sync;

    struct async_t {
        explicit async_t() = default;
    };

    extern async_t const async;

}

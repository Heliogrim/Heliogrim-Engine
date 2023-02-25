#pragma once

#include <Engine.Common/Types.hpp>

namespace ember::engine::utils {
    __interface Dispatchable {
        /**
         * Gets the batch count
         *
         * @author Julius
         * @date 14.05.2020
         *
         * @returns An uint32_t.
         */
        u32 batches() const;

        /**
         * Before the invocation
         *
         * @author Julius
         * @date 14.05.2020
         */
        void before();

        /**
         * Executes the given operation on a different thread, and waits for the result
         *
         * @author Julius
         * @date 14.05.2020
         *
         * @param  index_ Zero-based index of the.
         */
        void invoke(const u32 index_);

        /**
         * After the invocation
         *
         * @author Julius
         * @date 14.05.2020
         */
        void after();
    };
}

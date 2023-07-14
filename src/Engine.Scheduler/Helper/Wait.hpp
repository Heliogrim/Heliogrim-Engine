#pragma once

#include <atomic>

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Types.hpp>

namespace hg::engine::scheduler {
    /**
     * Wait on a atomic variable
     *
     * @details This function will determine the current execution context
     *  and use a proper way to wait on an atomic integral with an expectation.
     *
     * @param atomic_ The variable holding the atomic value.
     * @param prev_ The value which should change.
     */
    void waitOnAtomic(cref<_STD atomic_flag> atomic_, const bool prev_);

    /**
     * Wait on a atomic variable
     *
     * @details This function will determine the current execution context
     *  and use a proper way to wait on an atomic integral with an expectation.
     *
     * @param atomic_ The variable holding the atomic value.
     * @param prev_ The value which should change.
     */
    void waitOnAtomic(cref<_STD atomic_uint_fast8_t> atomic_, const u8 prev_);

    /**
     * Wait until a atomic variable matches the expectation
     *
     * @details This function will determine the current execution context
     *  and use a proper way to wait on an atomic integral with an expectation.
     *
     * @param atomic_ The variable holding the atomic value.
     * @param expect_ The value to wait for.
     */
    void waitUntilAtomic(cref<_STD atomic_flag> atomic_, const bool expect_);

    /**
     * Wait until a atomic variable matches the expectation
     *
     * @details This function will determine the current execution context
     *  and use a proper way to wait on an atomic integral with an expectation.
     *
     * @param atomic_ The variable holding the atomic value.
     * @param expect_ The value to wait for.
     */
    void waitUntilAtomic(cref<_STD atomic_uint_fast8_t> atomic_, const u8 expect_);
}

#pragma once

#include <Windows.h>
#include <Engine.Common/Wrapper.hpp>

#include "../Task/Task.hpp"

namespace ember::engine::scheduler::fiber {

    struct Fiber {
    public:
        using handle_type = ptr<void>;

    public:
        handle_type handle;

        handle_type parent;

        task::__TaskDelegate task;

    public:
        /**
         * Destroys this fiber
         *
         * @author Julius
         * @date 21.10.2021
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool destroy();

        /**
         * Destroys this fiber
         *  Warning: You should guarantee, that you don't call from fiber
         *
         * @author Julius
         * @date 22.10.2021
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool destroy_external();

        /**
         * Yields this fiber
         *
         * @author Julius
         * @date 21.10.2021
         */
        void yield();
    };

    namespace self {

        /**
         * Yields the current fiber
         *
         * @author Julius
         * @date 21.10.2021
         */
        void yield();
    }

    /**
     * Get the intended default fiber stack size in bytes
     *
     * @author Julius
     * @date 22.10.2021
     *
     * @returns An u64.
     */
    extern constexpr u64 default_fiber_stack_size() noexcept;

}

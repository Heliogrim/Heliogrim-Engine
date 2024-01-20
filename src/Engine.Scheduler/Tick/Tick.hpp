#pragma once

#include <atomic>
#include <Engine.Common/Types.hpp>
#include "TickTaskQueue.hpp"
#include "../Task/Task.hpp"

namespace clockwork::__internal::core::tick {
    class Tick {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 13.11.2020
         */
        Tick() noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 13.11.2020
         */
        ~Tick() = default;

        /**
         * Query if this is blocked
         *
         * @author Julius
         * @date 13.11.2020
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool blocked() const noexcept;

        /**
         * Adds task_
         *
         * @author Julius
         * @date 13.11.2020
         *
         * @param  task_ The task to add.
         */
        void add(scheduler::task::__TaskDelegate task_);

    private:
        /** The tasks */
        TickTaskQueue _tasks;

        /**
         * Gets the number of blocking tasks
         */
        _STD atomic<hg::u32> _blockCount;
    };
}

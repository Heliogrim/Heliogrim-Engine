#pragma once

#include <Engine.Common/Wrapper.hpp>
#include "Thread.hpp"
#include "../Task/SharedQueue.hpp"

namespace ember::engine::scheduler::thread {

    class Worker final {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 14.11.2020
         *
         * @param  queue_ The queue.
         * @param  mask_ The mask.
         */
        Worker(IN ptr<task::SharedQueue> queue_, IN task::TaskMask mask_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 14.11.2020
         */
        ~Worker();

        /**
         * Copy - Constructor
         *
         * @author Julius
         * @date 14.11.2020
         *
         * @param  other_ The other.
         */
        Worker(const Worker& other_) = delete;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 14.11.2020
         *
         * @param  other_ The other.
         */
        Worker(const Worker&& other_) = delete;

        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 14.11.2020
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        Worker& operator=(const Worker& other_) = delete;

        /**
         * Move assignment operator
         *
         * @author Julius
         * @date 14.11.2020
         *
         * @param [in,out] other_ The other.
         *
         * @returns A shallow copy of this.
         */
        Worker& operator=(Worker&& other_) = delete;

        /**
         * Starts this 
         *
         * @author Julius
         * @date 14.11.2020
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool start();

        /**
         * Stops this 
         *
         * @author Julius
         * @date 22.07.2021
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool stop();

        /**
         * Joins this 
         *
         * @author Julius
         * @date 14.11.2020
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool join();

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 14.11.2020
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool destroy();

        /**
         * Gets the queue
         *
         * @author Julius
         * @date 15.12.2020
         *
         * @returns A ptr&lt;task::SharedQueue&gt;
         */
        [[nodiscard]] ptr<task::SharedQueue> queue() const noexcept;

        /**
         * Gets the mask
         *
         * @author Julius
         * @date 16.11.2020
         *
         * @returns A task::TaskMask.
         */
        [[nodiscard]] task::TaskMask mask() const;

    private:
        /** The interrupt pointer */
        ptr<volatile bool> _interrupt_ptr = nullptr;

    public:
        /**
         * Sets interrupt pointer
         *
         * @author Julius
         * @date 22.07.2021
         *
         * @param  pointer_ The pointer.
         */
        void setInterruptPtr(ptr<volatile bool> pointer_) noexcept;

    private:
        ptr<task::SharedQueue> _queue;
        Thread _thread;
        task::TaskMask _mask;
    };
}

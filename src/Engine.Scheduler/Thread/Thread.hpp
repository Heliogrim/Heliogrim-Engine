#pragma once

#include <cassert>
#include <functional>
#include <thread>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/__macro.hpp>

namespace hg::engine::scheduler::thread {
    using thread_id = std::conditional_t<sizeof(std::thread::id) == sizeof(u64), u64, u32>;
    typedef u64 affinity_mask;

    typedef std::thread* thread_handle;

    enum priority: u8 {
        eTimeCritical = 127u,
        eHigh = 32u,
        eNormal = 8u,
        eLow = 0u
    };

    class Thread final {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 14.11.2020
         */
        Thread();

        /**
         * Destructor
         *
         * @author Julius
         * @date 14.11.2020
         */
        ~Thread();

        /**
         * Starts this
         *
         * @author Julius
         * @date 14.11.2020
         *
         * @param [in,out] fnc_ The function.
         * @param [in]     args_ If non-null, the arguments.
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool start(const std::function<void(void*)>& fnc_, IN void* args_);

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
        bool destroy();

        /**
         * Gets the identifier
         *
         * @author Julius
         * @date 14.11.2020
         *
         * @returns The identifier.
         */
        [[nodiscard]] thread_id getId() const noexcept;

        /**
         * Sets an affinity
         *
         * @author Julius
         * @date 14.11.2020
         *
         * @param  mask_ The mask.
         *
         * @returns An affinity_mask.
         */
        affinity_mask setAffinity(affinity_mask mask_);

        /**
         * Sets a priority
         *
         * @author Julius
         * @date 14.11.2020
         *
         * @param  priority_ The priority.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool setPriority(priority priority_);

        /**
         * Sets a name
         *
         * @author Julius
         * @date 14.11.2020
         *
         * @param  name_ The name.
         */
        void setName(cstr name_);

    private:
        thread_handle _handle;
    };

    namespace self {
        /**
         * Yields this 
         *
         * @author Julius
         * @date 14.11.2020
         */
        void yield();

        /**
         * Sleep for
         *
         * @author Julius
         * @date 14.11.2020
         *
         * @param  milliseconds_ The milliseconds.
         */
        void sleepFor(u64 milliseconds_);

        /**
         * Gets the identifier
         *
         * @author Julius
         * @date 14.11.2020
         *
         * @returns The identifier.
         */
        [[nodiscard]] thread_id getId() noexcept;

        /**
         * Get the process thread index
         *  Warning: This is not guaranteed unique. It is just intended for speedup purpose using sparse start indexes for mapped access.
         *
         * @author Julius
         * @date 17.11.2021
         *
         * @returns The index of this thread.
         */
        [[nodiscard]] u64 getIdx() noexcept;
    }

    /**
     * Gets native thread count
     *
     * @author Julius
     * @date 14.11.2020
     *
     * @returns The native thread count.
     */
    u32 getNativeThreadCount();
}

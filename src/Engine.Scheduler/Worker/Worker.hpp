#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "../Fiber/Fiber.hpp"
#include "../Fiber/FiberPool.hpp"
#include "../Process/Schedule.hpp"
#include "../Thread/Thread.hpp"

namespace hg::engine::scheduler::worker {
	class Worker final {
	public:
		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 14.11.2020
		 *
		 * @param schedule_ The schedule to work with.
		 * @param fiberPool_ The fiber pool.
		 * @param mask_ The mask.
		 */
		Worker(
			_In_ non_owning_rptr<Schedule> schedule_,
			_In_ ptr<fiber::FiberPool> fiberPool_,
			_In_ task::TaskMask mask_
		) noexcept;

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
		 * Check whether worker is ready
		 *
		 * @author Julius
		 * @date 20.03.2022
		 *
		 * @returns True if this is ready, otherwise false.
		 */
		[[nodiscard]] bool ready() const noexcept;

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
		 * Gets the underlying schedule
		 *
		 * @author Julius
		 * @date 15.12.2020
		 *
		 * @returns A const pointer to a schedule object.
		 */
		[[nodiscard]] const non_owning_rptr<Schedule> schedule() const noexcept;

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

	private:
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
		fiber::Fiber::handle_type _fiber;

	private:
		/**
		 * Sets fiber handle
		 *
		 * @author Julius
		 * @date 21.10.2021
		 *
		 * @param  fiber_ The fiber.
		 */
		void setFiberHandle(fiber::Fiber::handle_type fiber_) noexcept;

	private:
		ptr<fiber::FiberPool> _fiberPool;

	public:
		/**
		 * Get the fiber pool
		 *
		 * @author Julius
		 * @date 21.10.2021
		 *
		 * @returns A ptr&lt;fiber::FiberPool&gt;
		 */
		[[nodiscard]] ptr<fiber::FiberPool> fiberPool() const noexcept;

	private:
		non_owning_rptr<Schedule> _schedule;
		thread::Thread _thread;
		task::TaskMask _mask;

	private:
		static void handle(void* args_);
	};
}

#pragma once

#include <Engine.Asserts/Panic.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Cancellation.hpp"
#include "../Waitable/ParkQueue.hpp"
#include "../Task/Task.hpp"

namespace hg::engine::scheduler {
	class Schedule;
}

namespace hg::engine::scheduler::fiber {
	struct Fiber {
	public:
		using handle_type = ptr<void>;

	public:
		/**
		 * Context Handling
		 */
		handle_type handle = nullptr;

		handle_type parent = nullptr;

	public:
		/**
		 * Execution Model
		 */
		non_owning_rptr<const task::TaskDelegate> task = nullptr;

		ptr<Schedule> schedule = nullptr;

		ParkNode parkNode;

		CancellationToken cancellation;

	public:
		#if defined(WIN32)
		/**
		 * Creates a new ptr&lt;Fiber&gt;
		 *
		 * @author Julius
		 * @date 23.10.2021
		 *
		 * @param      self_ The self.
		 * @param [in] proc_ If non-null, the procedure.
		 */
		static void create(_Inout_ ptr<Fiber> self_, _In_ void (*proc_)(void*));

		#else
		/**
		 * Creates a new ptr&lt;Fiber&gt;
		 *
		 * @author Julius
		 * @date 23.10.2021
		 *
		 * @param      self_ The self.
		 * @param [in] proc_ If non-null, the procedure.
		 */
		static void create(
			_Inout_ ptr<Fiber> self_,
			_In_ void (*proc_)(int ptrLow_, int ptrHigh_)
		);

		#endif

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
		 * Yields this fiber
		 *
		 * @author Julius
		 * @date 21.10.2021
		 */
		void yield();
	};

	namespace self {
		/**
		 * The fiber running the calling task, null on a plain thread or a worker's root fiber
		 */
		[[nodiscard]] ptr<Fiber> current() noexcept;

		/**
		 * Yields the current fiber
		 *
		 * @author Julius
		 * @date 21.10.2021
		 */
		void yield();

		/**
		 * Whether the task running on the current fiber was asked to cancel (cooperative, never interrupts)
		 */
		[[nodiscard]] bool cancellationRequested() noexcept;
	}

	/**
	 * Get the intended default fiber stack size in bytes
	 *
	 * @author Julius
	 * @date 22.10.2021
	 *
	 * @returns An u64.
	 */
	extern u64 default_fiber_stack_size() noexcept;

	/**
	 * Switch thread to fiber
	 *
	 * @author Julius
	 * @date 23.10.2021
	 */
	extern void switch_thread_to_fiber();

	/**
	 * Switch fiber to thread
	 *
	 * @author Julius
	 * @date 23.10.2021
	 */
	extern void switch_fiber_to_thread();
}

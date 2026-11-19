#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/__macro.hpp>

#include "../Lane.hpp"
#include "../Task/Task.hpp"
#include "../Waitable/ParkQueue.hpp"
#include "Cancellation.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::scheduler {
	class Schedule;
	struct BoostCell;
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

		/**
		 * The cell the running task produces into; boosts raise it. Cleared with task. Raw: the bound task keeps the
		 * cell alive while it runs or is parked, and every re-push passes it to pushBoostable while still owning the task.
		 */
		ptr<BoostCell> boost = nullptr;

		ParkNode parkNode;

		CancellationToken cancellation;

		/** Stamped by the worker right before each launch or resume. */
		std::chrono::steady_clock::time_point sliceStart {};

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

		/**
		 * Binds the cell the running task produces into; its lane counts toward effectiveLane and every re-push.
		 */
		void bindBoost(ptr<BoostCell> cell_) noexcept;

		/**
		 * The cell bound to the running task, null if none or off a fiber
		 */
		[[nodiscard]] ptr<BoostCell> boostCell() noexcept;

		/**
		 * The lane the running task counts as: the higher of its own and its bound cell's.
		 */
		[[nodiscard]] Lane effectiveLane() noexcept;

		/**
		 * Yields when the running task is in a lower lane and has used its slice, or at once when a boost raised it
		 * above its queued lane (the re-push then queues it in the raised lane); no-op elsewhere
		 */
		void checkpoint();
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

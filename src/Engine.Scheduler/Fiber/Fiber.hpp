#pragma once

#include <Engine.Asserts/Panic.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Awaitable.hpp"
#include "../Task/Task.hpp"

namespace hg::engine::scheduler::fiber {
	struct Fiber {
	public:
		using handle_type = ptr<void>;

	public:
		/**
		 * Context Handling
		 */
		handle_type handle;

		handle_type parent;

	public:
		/**
		 * Execution Model
		 */
		non_owning_rptr<const task::TaskDelegate> task;

		FiberAwaitable awaiter;

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

		/**
		 * Awaits the given awaitable_ with this fiber
		 *
		 * @author Julius
		 * @date 23.10.2021
		 *
		 * @param  awaitable_ The awaitable.
		 */
		void await(_Inout_ mref<FiberAwaitable> awaitable_);
	};

	namespace self {
		/**
		 * Yields the current fiber
		 *
		 * @author Julius
		 * @date 21.10.2021
		 */
		void yield();

		/**
		 * Awaits the given awaitable_ with current fiber
		 *
		 * @author Julius
		 * @date 23.10.2021
		 *
		 * @param  awaitable_ The awaitable.
		 */
		extern void await(_Inout_ mref<FiberAwaitable> awaitable_);

		template <IsAwaitableSignal AwaitableType_>
		void await_signal(_In_ const ptr<AwaitableType_> awaitable_) {
			await(
				{
					.mask = FiberAwaitableBits::eSignal,
					.signal = awaitable_
				}
			);
		}

		template <IsAwaitableSignalCall AwaitableType_>
		void await_signal_call(cref<AwaitableType_> awaitable_) {
			await(
				{
					.mask = FiberAwaitableBits::eSignalCall,
					.call = awaitable_
				}
			);
		}

		template <IsAwaitable AwaitableType_>
		void await(
			_In_ std::conditional_t<IsAwaitableSignal<AwaitableType_>, const ptr<AwaitableType_>, cref<AwaitableType_>>
			awaitable_
		) {
			if constexpr (IsAwaitableSignal<AwaitableType_>) {
				return await_signal<AwaitableType_>(awaitable_);
			} else if constexpr (IsAwaitableSignalRet<AwaitableType_>) {
				return await_signal<await_signal_sub_type>(awaitable_.await());
			} else if constexpr (IsAwaitableSignalCall<AwaitableType_>) {
				return await_signal_call<AwaitableType_>(awaitable_);
			} else {
				::hg::panic();
			}
		}
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

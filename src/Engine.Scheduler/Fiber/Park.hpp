#pragma once

#include "Fiber.hpp"
#include "../Waitable/ParkQueue.hpp"

namespace hg::engine::scheduler {
	/**
	 * The fiber's embedded node (fiber set), or this plain thread's node (fiber null).
	 * Callers arm the node, push it into a queue, then call park().
	 */
	[[nodiscard]] ref<ParkNode> currentParkNode() noexcept;

	/**
	 * Fiber: yields to the worker and returns once unparked. Thread: blocks on threadFlag.
	 * Leaves node_ in eIdle.
	 */
	void park(ref<ParkNode> node_);

	// Callers must read node_.next before calling this; the waiter may reuse the node immediately.
	void unpark(ref<ParkNode> node_) noexcept;

	#ifdef _DEBUG
	namespace worker {
		using PostSwitchHook = void(*)(ptr<fiber::Fiber>);
		extern PostSwitchHook debugPostSwitchHook;
	}
	#endif
}

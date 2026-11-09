#pragma once

#include <atomic>
#include <Engine.Common/Types.hpp>

#include "ParkQueue.hpp"

namespace hg::engine::scheduler {
	/**
	 * One-shot signal; waiters park (fiber or thread) until set() is called once.
	 * Destroying a Signal is safe once isSet() returned true or wait() returned;
	 * the setter's _state store is its last access to the object.
	 */
	class Signal {
	public:
		[[nodiscard]] bool isSet() const noexcept;

		void set() noexcept;

		void wait();

		// Re-arms the one-shot; must not run concurrently with set() or wait(),
		// and only while no waiter is queued.
		void reset() noexcept;

	private:
		std::atomic<u32> _state { 0 };
		ParkQueue _waiters;
		// Read and written only under the _waiters lock.
		bool _closed = false;
	};
}

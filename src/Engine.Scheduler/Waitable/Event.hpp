#pragma once

#include <atomic>
#include <Engine.Common/Types.hpp>

#include "ParkQueue.hpp"

namespace hg::engine::scheduler {
	/**
	 * One-shot event; waiters park (fiber or thread) until set() is called once.
	 */
	class Event {
	public:
		[[nodiscard]] bool isSet() const noexcept;

		void set() noexcept;

		void wait();

	private:
		std::atomic<u32> _state { 0 };
		ParkQueue _waiters;
	};
}

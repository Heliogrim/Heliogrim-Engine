#pragma once

#include <atomic>
#include <Engine.Common/Types.hpp>

#include "Event.hpp"

namespace hg::engine::scheduler {
	/**
	 * Fork-join counter; the creator holds one guard so children finishing early cannot complete it.
	 */
	class JoinCounter {
	public:
		explicit JoinCounter(u32 children_) noexcept;

		/** Must be called before the matching children start. */
		void add(u32 n_ = 1) noexcept;

		void done() noexcept;

		void wait();

		[[nodiscard]] bool isDone() const noexcept;

	private:
		std::atomic<u32> _pending;
		Event _done;
		bool _waited = false;
	};
}

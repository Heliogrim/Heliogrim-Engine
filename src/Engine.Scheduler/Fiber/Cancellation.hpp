#pragma once

#include <atomic>
#include <Engine.Common/Managed/Rc.hpp>

namespace hg::engine::scheduler::fiber {
	/**
	 * @note: Not std::stop_source: poll-only, shared via Arci with Task/Fiber, and no stop_callback whose dtor blocks the worker thread.
	 */
	class CancellationSource :
		public ArcFromThis<CancellationSource> {
	public:
		void cancel() noexcept;

		[[nodiscard]] bool isCancellationRequested() const noexcept;

	private:
		std::atomic<bool> _requested { false };
	};

	using CancellationToken = Arci<const CancellationSource>;
}

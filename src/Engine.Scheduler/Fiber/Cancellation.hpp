#pragma once

#include <atomic>
#include <Engine.Common/Managed/Rc.hpp>

namespace hg::engine::scheduler::fiber {
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

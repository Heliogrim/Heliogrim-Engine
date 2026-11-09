#include "Cancellation.hpp"

using namespace hg::engine::scheduler::fiber;

void CancellationSource::cancel() noexcept {
	_requested.store(true, std::memory_order::release);
}

bool CancellationSource::isCancellationRequested() const noexcept {
	return _requested.load(std::memory_order::acquire);
}

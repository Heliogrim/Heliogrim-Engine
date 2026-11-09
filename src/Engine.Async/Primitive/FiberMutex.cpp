#include "FiberMutex.hpp"

#include <immintrin.h>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Scheduler/Fiber/Park.hpp>

using namespace hg::concurrent;
using namespace hg::engine::scheduler;
using namespace hg;

bool FiberMutex::try_lock() noexcept {
	u32 w = _word.load(std::memory_order::relaxed);
	return (w & eLocked) == 0 && _word.compare_exchange_strong(
		w,
		w | eLocked,
		std::memory_order::acquire,
		std::memory_order::relaxed
	);
}

void FiberMutex::lock() {
	for (;;) {
		for (u32 spin = 0; spin < spinLimit; ++spin) {
			if (try_lock()) {
				return;
			}
			_mm_pause();
		}

		auto& node = currentParkNode();
		bool acquired = false;

		const bool pushed = _waiters.locked(
			[&](auto locked_) {
				u32 w = _word.load(std::memory_order::relaxed);
				if ((w & eLocked) == 0) {
					acquired = _word.compare_exchange_strong(
						w,
						w | eLocked,
						std::memory_order::acquire,
						std::memory_order::relaxed
					);
					return false;
				}
				// A failed CAS means the word moved (e.g. unlock); fetch_or could set eContended on a free mutex nobody will wake.
				if (not _word.compare_exchange_strong(w, w | eContended, std::memory_order::relaxed)) {
					return false;
				}
				node.state.store(ParkNode::State::eArmed, std::memory_order::relaxed);
				locked_.push(node);
				return true;
			}
		);

		if (acquired) {
			return;
		}
		if (pushed) {
			park(node);
		}
	}
}

void FiberMutex::unlock() noexcept {
	const u32 prev = _word.exchange(0, std::memory_order::release);
	::hg::assertd((prev & eLocked) != 0);

	if ((prev & eContended) == 0) {
		return;
	}

	ptr<ParkNode> node = _waiters.locked(
		[&](auto locked_) {
			auto* const head = locked_.popOne();
			if (not locked_.empty()) {
				// A barger may already hold eLocked, so a store would drop it.
				_word.fetch_or(eContended, std::memory_order::relaxed);
			}
			return head;
		}
	);

	if (node != nullptr) {
		unpark(*node);
	}
}

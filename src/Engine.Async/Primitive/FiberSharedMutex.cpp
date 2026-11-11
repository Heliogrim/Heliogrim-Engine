#include "FiberSharedMutex.hpp"

#include <immintrin.h>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Scheduler/Fiber/Park.hpp>

using namespace hg::concurrent;
using namespace hg::engine::scheduler;
using namespace hg;

namespace {
	// Lock order is always readers then writers; together the two guards form the single queue lock.
	template <typename Fn_>
	decltype(auto) lockBoth(ref<ParkQueue> readers_, ref<ParkQueue> writers_, Fn_&& fn_) {
		return readers_.locked(
			[&](auto readersLocked_) {
				return writers_.locked(
					[&](auto writersLocked_) {
						return ::hg::forward<Fn_>(fn_)(readersLocked_, writersLocked_);
					}
				);
			}
		);
	}
}

bool FiberSharedMutex::try_lock() noexcept {
	u32 expected = 0;
	return _word.compare_exchange_strong(expected, eWriter, std::memory_order::acquire, std::memory_order::relaxed);
}

void FiberSharedMutex::lock() {
	for (u32 spin = 0; spin < spinLimit; ++spin) {
		if (try_lock()) {
			return;
		}
		_mm_pause();
	}

	auto& node = currentParkNode();
	const bool pushed = lockBoth(
		_readers,
		_writers,
		[&](auto, auto writers_) {
			u32 w = _word.fetch_or(eWriterWaiting, std::memory_order::acq_rel) | eWriterWaiting;
			// Queued writers go first: a free lock with a non-empty queue is handed over by whoever freed it.
			// A transient reader count can fail the CAS; it then reaches 0 under eWriterWaiting and hands off to us.
			while (writers_.empty() && (w & (eWriter | readerMask)) == 0) {
				if (_word.compare_exchange_weak(w, eWriter, std::memory_order::acquire, std::memory_order::relaxed)) {
					return false;
				}
			}

			node.state.store(ParkNode::State::eArmed, std::memory_order::relaxed);
			writers_.push(node);
			return true;
		}
	);

	if (pushed) {
		// Handoff: the waker set eWriter on our behalf.
		park(node);
	}
}

void FiberSharedMutex::unlock() noexcept {
	::hg::assertd((_word.load(std::memory_order::relaxed) & eWriter) != 0);

	ptr<ParkNode> batch = nullptr;
	ptr<ParkNode> writer = nullptr;

	lockBoth(
		_readers,
		_writers,
		[&](auto readers_, auto writers_) {
			// Alternate between the queues so neither side starves under a steady stream of the other.
			if (not readers_.empty() && (writers_.empty() || not _readersLast)) {
				_readersLast = true;
				batch = readers_.takeAll();
				u32 n = 0;
				for (auto* it = batch; it != nullptr; it = it->next) {
					++n;
				}

				// Clears eWriter and admits the whole batch in one step; eWriterWaiting stays.
				_word.fetch_sub(eWriter - n, std::memory_order::acq_rel);
				return;
			}

			if (not writers_.empty()) {
				_readersLast = false;
				writer = writers_.popOne();
				if (writers_.empty()) {
					_word.fetch_and(~eWriterWaiting, std::memory_order::acq_rel);
				}
				return;
			}

			_word.fetch_and(~eWriter, std::memory_order::release);
		}
	);

	while (batch != nullptr) {
		auto* const next = batch->next;
		unpark(*batch);
		batch = next;
	}

	if (writer != nullptr) {
		unpark(*writer);
	}
}

bool FiberSharedMutex::try_lock_shared() noexcept {
	u32 w = _word.load(std::memory_order::relaxed);
	while ((w & writerBits) == 0) {
		if (_word.compare_exchange_weak(w, w + 1, std::memory_order::acquire, std::memory_order::relaxed)) {
			return true;
		}
	}
	return false;
}

// The slow path drops the transient reader count taken above before parking.
void FiberSharedMutex::lock_shared() HG_NO_THREAD_SAFETY_ANALYSIS {

	const u32 prev = _word.fetch_add(1, std::memory_order::acquire);
	if ((prev & writerBits) == 0) {
		return;
	}
	unlock_shared();

	auto& node = currentParkNode();
	const bool pushed = lockBoth(
		_readers,
		_writers,
		[&](auto readers_, auto) HG_NO_THREAD_SAFETY_ANALYSIS {
			// CAS instead of fetch_add: a fast-path writer may take 0 -> eWriter without the queue lock.
			if (try_lock_shared()) {
				return false;
			}

			node.state.store(ParkNode::State::eArmed, std::memory_order::relaxed);
			readers_.push(node);
			return true;
		}
	);

	if (pushed) {
		// Handoff: the waking writer already added our reader count.
		park(node);
	}
}

void FiberSharedMutex::unlock_shared() noexcept {
	const u32 prev = _word.fetch_sub(1, std::memory_order::acq_rel);
	::hg::assertd((prev & readerMask) != 0);

	if ((prev & readerMask) != 1 || (prev & eWriterWaiting) == 0) {
		return;
	}

	ptr<ParkNode> writer = lockBoth(
		_readers,
		_writers,
		[&](auto, auto writers_) -> ptr<ParkNode> {
			// A transient fast-path reader may hold a count; whichever fetch_sub reaches 0 last re-runs this check.
			const u32 w = _word.load(std::memory_order::acquire);
			if ((w & (eWriter | readerMask)) != 0) {
				return nullptr;
			}

			auto* const head = writers_.popOne();
			if (head == nullptr) {
				return nullptr;
			}

			_word.fetch_or(eWriter, std::memory_order::acq_rel);
			if (writers_.empty()) {
				_word.fetch_and(~eWriterWaiting, std::memory_order::acq_rel);
			}

			return head;
		}
	);

	if (writer != nullptr) {
		unpark(*writer);
	}
}

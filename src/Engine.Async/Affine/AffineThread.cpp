#include "AffineThread.hpp"

#include <bit>
#include <exception>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/String/Convert.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>

#ifdef WIN32
#include <Engine.Common/stdafx.h>
/**/
#include <processthreadsapi.h>
#endif

using namespace hg::engine::async;
using namespace hg;

thread_local static ptr<AffineThread> __currentAffine { nullptr };

AffineThread::AffineThread(string name_, mref<AffineWaker> waker_) :
	_name(std::move(name_)) {
	_wakers.emplace_back(make_uptr<AffineWaker>(std::move(waker_)));
	_waker.store(_wakers.back().get(), std::memory_order::release);
}

AffineThread::~AffineThread() {
	::hg::assertd(not _thread.joinable());

	// Leftover items (never started, or the loop gave up) drop their promises.
	auto list = std::bit_cast<ptr<Node>>(_head.exchange(closed_bit, std::memory_order::acquire) & ~closed_bit);
	while (list != nullptr) {
		delete std::exchange(list, list->next);
	}
	while (_pending != nullptr) {
		delete std::exchange(_pending, _pending->next);
	}
}

void AffineThread::start(mref<UniqueFn<void(ref<AffineThread>)>> loop_) {
	::hg::assertd(not _thread.joinable());
	::hg::assertd(not stopping());

	_thread = std::thread {
		[this, loop = std::move(loop_)]() mutable {
			#ifdef WIN32
			const auto name = stringToWString(_name);
			SetThreadDescription(GetCurrentThread(), name.c_str());
			#endif

			scheduler::thread::self::markAffine(true);
			__currentAffine = this;

			std::move(loop)(*this);

			__currentAffine = nullptr;
			scheduler::thread::self::markAffine(false);
		}
	};
	// Only after the thread exists: stop() joins whenever _started is set.
	_started = true;
}

void AffineThread::drain() {
	::hg::assertd(isCurrent());

	for (;;) {
		// Take new items only once the unrun remainder is done, so FIFO holds across nested drains.
		if (_pending == nullptr) {
			// acq_rel: acquire for node contents; release so a producer reading this empty head also sees a prior setWaker.
			auto list = std::bit_cast<ptr<Node>>(_head.fetch_and(closed_bit, std::memory_order::acq_rel) & ~closed_bit);
			if (list == nullptr) {
				return;
			}

			ptr<Node> fifo = nullptr;
			while (list != nullptr) {
				auto* const next = list->next;
				list->next = fifo;
				fifo = list;
				list = next;
			}
			_pending = fifo;
		}

		auto* const node = std::exchange(_pending, _pending->next);
		auto item = std::move(node->fn);
		delete node;
		if (_depth.fetch_sub(1uLL, std::memory_order::relaxed) - 1uLL <= depth_rearm && not _depthArmed.load(std::memory_order::relaxed)) {
			_depthArmed.store(true, std::memory_order::relaxed);
		}

		// A throwing item already destroyed its closure, which drops its promise (eDropped).
		try {
			std::move(item)();
		} catch (const std::exception& exc_) {
			IM_CORE_ERRORF("Affine thread `{}` item threw: {}", _name, exc_.what());
		} catch (...) {
			IM_CORE_ERRORF("Affine thread `{}` item threw an unknown exception.", _name);
		}
	}
}

bool AffineThread::stopping() const noexcept {
	return (_head.load(std::memory_order::acquire) & closed_bit) != 0uLL;
}

void AffineThread::stop() {
	::hg::assertd(not isCurrent());

	// acq_rel: orders the close against a concurrent setWaker store, so the wake below sees the newest waker or the loop sees the close.
	_head.fetch_or(closed_bit, std::memory_order::acq_rel);
	// Unconditional wake: the loop must observe stopping() even when the inbox is empty.
	wake();

	if (not _started) {
		return;
	}
	if (_joining.exchange(true, std::memory_order::acq_rel)) {
		// Another caller is joining; return only once the loop thread is really gone.
		_joined.wait(false, std::memory_order::acquire);
		return;
	}
	_thread.join();
	_joined.store(true, std::memory_order::release);
	_joined.notify_all();
}

void AffineThread::setWaker(mref<AffineWaker> waker_) {
	::hg::assertd(isCurrent());
	_wakers.emplace_back(make_uptr<AffineWaker>(std::move(waker_)));
	_waker.store(_wakers.back().get(), std::memory_order::release);
}

u64 AffineThread::depth() const noexcept {
	return _depth.load(std::memory_order::relaxed);
}

ptr<AffineThread> AffineThread::current() noexcept {
	return __currentAffine;
}

bool AffineThread::isCurrent() const noexcept {
	return __currentAffine == this;
}

bool AffineThread::push(mref<Item> item_) {
	auto* const node = new Node { std::move(item_) };

	// Counted before the CAS, so the consumer's decrement never precedes it.
	const auto prev = _depth.fetch_add(1uLL, std::memory_order::relaxed);

	auto head = _head.load(std::memory_order::relaxed);
	do {
		if ((head & closed_bit) != 0uLL) {
			_depth.fetch_sub(1uLL, std::memory_order::relaxed);
			delete node;
			return false;
		}

		node->next = std::bit_cast<ptr<Node>>(head);
	} while (not _head.compare_exchange_weak(
		head,
		std::bit_cast<std::uintptr_t>(node),
		// acq_rel, not release: the waker load in wake() must not pass this RMW (store-buffer race with setWaker + drain).
		std::memory_order::acq_rel,
		std::memory_order::relaxed
	));

	// Warn once on reaching the threshold; drain re-arms below depth_rearm, so hovering at the line stays quiet.
	if (prev + 1uLL >= depth_warning && _depthArmed.exchange(false, std::memory_order::relaxed)) {
		_warnings.fetch_add(1uLL, std::memory_order::relaxed);
		IM_CORE_WARNF("Affine thread `{}` inbox crossed {} queued items.", _name, depth_warning);
	}

	// Only the empty -> non-empty transition wakes; later pushes find the loop already signalled.
	if (head == 0uLL) {
		wake();
	}
	return true;
}

void AffineThread::wake() const {
	const auto* const waker = _waker.load(std::memory_order::acquire);
	if (waker != nullptr && *waker) {
		(*waker)();
	}
}

void AffineThread::warnRejected() {
	if (not _rejectWarned.exchange(true, std::memory_order::relaxed)) {
		IM_CORE_WARNF("Affine thread `{}` is stopped; detached item destroyed unrun.", _name);
	}
}

#ifdef _DEBUG
u64 AffineThread::warningsIssued() const noexcept {
	return _warnings.load(std::memory_order::relaxed);
}
#endif

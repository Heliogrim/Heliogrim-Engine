#include "Worker.hpp"

#include <chrono>
#include <stdexcept>
#include <Engine.Common/stdafx.h>

#if not defined(WIN32)
#include <ucontext.h>
#include <signal.h>
#endif

#include "../Fiber/FiberLaunchPad.hpp"
#include "../Fiber/FiberPool.hpp"
#include "../Fiber/Park.hpp"
import Engine.Asserts;

using namespace hg::engine::scheduler::worker;
using namespace hg::engine::scheduler::thread;
using namespace hg::engine::scheduler;
using namespace hg;

Worker::Worker(
	const non_owning_rptr<Schedule> schedule_,
	const ptr<fiber::FiberPool> fiberPool_
) noexcept :
	_fiber(nullptr),
	_fiberPool(fiberPool_),
	_schedule(schedule_),
	_thread() {}

Worker::~Worker() {
	#ifdef _DEBUG
	assert(destroy());
	#else
	[[maybe_unused]] auto result { destroy() };
	#endif
};

bool Worker::start() {

	::hg::assertd(_fiber == nullptr /* "Worker seams to run already." */);
	::hg::assertd(_fiberPool != nullptr /* "Worker requires fiber pool before start." */);

	auto success = _thread.start(&Worker::handle, this);

	if (!success) {
		return false;
	}

	return true;
}

bool Worker::ready() const noexcept {
	return _ready.load(std::memory_order::acquire);
}

void Worker::requestStop() noexcept {
	_interrupt.store(true, std::memory_order::release);
}

bool Worker::join() {
	return _thread.join();
}

bool Worker::destroy() {
	return _thread.destroy();
}

const non_owning_rptr<Schedule> Worker::schedule() const noexcept {
	return _schedule;
}

void Worker::setFiberHandle(fiber::Fiber::handle_type fiber_) noexcept {
	_fiber = fiber_;
}

ptr<fiber::FiberPool> Worker::fiberPool() const noexcept {
	return _fiberPool;
}

void Worker::handle(void* args_) {

	/**
	 *
	 */
	Worker* worker { static_cast<Worker*>(args_) };

	/**
	 * Convert this thread to fiber
	 */
	#if defined(WIN32)
	auto* fiber { ConvertThreadToFiber(nullptr) };

	if (fiber == nullptr) {
		throw std::runtime_error("Could not convert this thread to fiber.");
	}

	#else
	auto* fiber = new ucontext_t {};

	#endif

	worker->setFiberHandle(fiber);

	/**
	 * Presolve used data
	 */
	auto* const schedule { worker->schedule() };

	/**
	 * Resolve required objects
	 */
	fiber::FiberPool& pool { *worker->_fiberPool };
	fiber::FiberLaunchPad launcher { nullptr };
	non_owning_rptr<const task::TaskDelegate> task = nullptr;
	Lane lane = Lane::eFrame;

	self::markWorker(true);
	worker->_ready.store(true, std::memory_order::release);

	while (not worker->_interrupt.load(std::memory_order::acquire)) [[likely]] {
		/**
		 * Load the epoch before the pop, so a push ordered after the pop moves it and the wait returns
		 */
		const auto epoch { schedule->epoch() };
		task = nullptr;

		if (schedule->pop(std::chrono::steady_clock::now(), task, lane)) {
			/**
			 * Cascade: wake two peers while more work is takeable
			 */
			schedule->cascade(std::chrono::steady_clock::now());

			launcher.self = task->fiber();

			/**
			 * Check for new task
			 */
			if (launcher.self == nullptr) {
				launcher.self = pool.acquire();
				launcher.self->task = task;
				launcher.self->boost = nullptr;
				launcher.self->schedule = schedule;
				launcher.self->cancellation = fiber::CancellationToken { task->cancellation() };
				const_cast<ptr<task::TaskDelegate>>(task)->fiber() = launcher.self;
			}

			::hg::assertd(launcher.self->parent == nullptr);
			::hg::assertd(launcher.self->task != nullptr);
			::hg::assertd(launcher.self->task->fiber() != nullptr);
			launcher.self->parent = fiber;
			const auto launchedAt = std::chrono::steady_clock::now();
			launcher.self->sliceStart = launchedAt;
			launcher();
			schedule->leave(lane);
			if (lane != Lane::eFrame) {
				const auto ran = std::chrono::steady_clock::now() - launchedAt;
				schedule->noteRun(lane, static_cast<u64>(std::chrono::duration_cast<std::chrono::microseconds>(ran).count()));
			}

			#ifdef _DEBUG
			if (debugPostSwitchHook != nullptr) {
				debugPostSwitchHook(launcher.self);
			}
			#endif

			/**
			 * Check whether task suspended execution
			 */
			if (launcher.self->task != nullptr) {
				/**
				 * Plain yield reschedules; a parked fiber is requeued by unpark, or here if unpark already ran
				 */
				auto& st = launcher.self->parkNode.state;
				if (st.load(std::memory_order::relaxed) == ParkNode::State::eIdle) {
					// quiet: this worker pops next
					(void)schedule->pushBoostable(task, launcher.self->boost, false);
				} else if (st.exchange(ParkNode::State::eParked, std::memory_order::acq_rel) == ParkNode::State::eWoken) {
					// quiet: this worker pops next
					(void)schedule->pushBoostable(task, launcher.self->boost, false);
				}

			} else {

				::hg::assertd(launcher.self->parent == nullptr);
				::hg::assertd(launcher.self->task == nullptr);

				/**
				 * Release execution context
				 */
				launcher.self->cancellation.reset();
				launcher.self->boost = nullptr;
				pool.release(std::move(launcher.self));
			}

		} else {
			if (worker->_interrupt.load(std::memory_order::acquire)) {
				break;
			}
			// bounded while lower work is queued (a floor only falls due for a worker that pops); skipped while a Frame push is in flight
			schedule->waitForWork(epoch, schedule->parkTimeoutMillis(std::chrono::steady_clock::now()));
		}
	}

	self::markWorker(false);

	/**
	 * Convert this fiber back to a thread and exit as intended
	 */
	#if defined(WIN32)
	[[maybe_unused]] const auto result = ConvertFiberToThread();
	::hg::assertd(result /* "Failed to convert fiber back to thread." */);

	#else
	delete fiber;

	#endif
}

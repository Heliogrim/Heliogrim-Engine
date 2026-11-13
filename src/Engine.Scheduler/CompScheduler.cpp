#include "CompScheduler.hpp"

#include <bit>
#include <cstdlib>
#include <utility>
#include <Engine.Asserts/Breakpoint.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Logging/Logger.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

namespace {
	/* Reserved for dedicated non-worker threads; raise when the audio device thread lands. */
	inline constexpr hg::u32 hot_thread_count = 0uL;
}

using namespace hg::engine::scheduler;
using namespace hg::engine;
using namespace hg;

CompScheduler::CompScheduler() noexcept :
	_workerCount(0),
	_workers(nullptr),
	_fiberPool(0),
	_pipeline(),
	_schedule() {}

CompScheduler::~CompScheduler() {
	destroy();
}

static u32 effectiveReserve(const u32 logical_, const u32 reserve_) noexcept {
	return reserve_ != 0uL ? reserve_ : static_cast<u32>(std::bit_width(logical_) - 1) + hot_thread_count;
}

u32 scheduler::autoWorkerCount(const u32 logical_, const u32 reserve_) noexcept {
	const auto reserve = effectiveReserve(logical_, reserve_);
	return logical_ > reserve ? logical_ - reserve : 1uL;
}

void CompScheduler::setup(u32 workers_, u32 reserve_) {
	SCOPED_STOPWATCH

	_schedule = make_uptr<Schedule>();
	_pipeline = make_uptr<CompositePipeline>(_schedule.get());

	/**/

	const char* source = workers_ != 0uL ? "config" : "auto";

	#ifdef USE_WORKER_RESERVE
	reserve_ = USE_WORKER_RESERVE;
	#endif
	#ifdef USE_WORKER_COUNT
	workers_ = USE_WORKER_COUNT;
	source = "macro";
	#endif

	const auto logical = static_cast<u32>(thread::getNativeThreadCount());
	if (workers_ == 0uL) {
		workers_ = autoWorkerCount(logical, reserve_);
	}

	IM_CORE_LOGF(
		"Scheduler starts {} workers ({} logical threads, reserve {}, source: {})",
		workers_,
		logical,
		effectiveReserve(logical, reserve_),
		source
	);

	/**
	 * Prepare pipeline and workers
	 */
	_schedule->setup();
	_workers = static_cast<aligned_worker*>(calloc(workers_, sizeof(aligned_worker)));

	/**
	 * Construct every worker with all available masks
	 */
	for (u32 i = 0; i < workers_; ++i) {
		::new(&_workers[i]) aligned_worker(_schedule.get(), &_fiberPool, task::TaskMask::eAll);
	}

	/**
	 * Start Workers
	 */
	for (u32 idx = 0; idx < workers_; ++idx) {
		#ifdef _DEBUG
		assert(_workers[idx].start());
		#else
		_workers[idx].start();
		#endif
	}

	_workerCount = workers_;
}

void CompScheduler::finalize() {
	_pipeline->setup();
}

void waitReadyAll(u32 workerCount_, const ptr<const CompScheduler::aligned_worker> workers_) {
	for (u32 i = 0; i < workerCount_; ++i) {
		while (!workers_[i].ready()) {
			#ifdef _DEBUG
			thread::self::sleepFor(5);
			#else
			thread::self::yield();
			#endif
		}
	}
}

void CompScheduler::destroy() {
	SCOPED_STOPWATCH

	if (_workerCount) {
		waitReadyAll(_workerCount, _workers);

		/**
		 * Destroy workers and transitive destroy threads
		 */
		for (u32 idx = 0; idx < _workerCount; ++idx) {
			#if _DEBUG
			assert(_workers[idx].stop());
			#else
			_workers[idx].stop();
			#endif
		}

		/**
		 * Destruct workers
		 */
		for (u32 idx = 0; idx < _workerCount; ++idx) {
			(&_workers[idx])->~Worker();
		}

		/**
		 * Free allocated memory
		 */
		free(_workers);

		/**
		 * Reset variable to be safe
		 */
		_workerCount = 0;
		_workers = nullptr;
	}

	/**/

	_pipeline.reset();
	_schedule.reset();
}

void CompScheduler::delay(mref<task_handle_type> task_, const u32 ticks_) {
	exec(std::move(task_));
}

void CompScheduler::exec(mref<task_handle_type> task_) {

	[[maybe_unused]] const auto result = _schedule->push(std::move(task_));

	#ifdef _DEBUG
	if (not result) {
		IM_CORE_ERROR("Failed to push task.");
		::hg::breakpoint();
	}
	#endif
}

const non_owning_rptr<CompositePipeline> CompScheduler::getCompositePipeline() const noexcept {
	return _pipeline.get();
}

bool CompScheduler::execOnStage(
	mref<task_handle_type> task_,
	const non_owning_rptr<const scheduler::Stage> stage_
) {
	_pipeline->pushTask(stage_, std::move(task_));
	return true;
}

bool CompScheduler::execOnStages(
	mref<task_handle_type> task_,
	const non_owning_rptr<const scheduler::Stage> begin_,
	const non_owning_rptr<const scheduler::Stage> end_
) {
	// TODO: Should get special behaviour <> Currently fast-forward
	_pipeline->pushTask(begin_, std::move(task_));
	return true;
}

size_t CompScheduler::getWorkerCount() const {
	return _workerCount;
}

void CompScheduler::wait() const {
	for (u32 i = 0; i < _workerCount; ++i) {
		#if _DEBUG
		assert(_workers[i].join());
		#else
		_workers[i].join();
		#endif
	}
}

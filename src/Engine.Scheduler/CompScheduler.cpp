#include "CompScheduler.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Logging/Logger.hpp>

#include "Tick/TickPipeline.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

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

void CompScheduler::setup(u32 workers_) {
    SCOPED_STOPWATCH

    _schedule = make_uptr<Schedule>();
    _pipeline = make_uptr<CompositePipeline>(_schedule.get());

    /**/

    #ifdef USE_WORKER_COUNT
    workers_ = USE_WORKER_COUNT;
    #endif

    workers_ = 1uL;

    if (workers_ == 0uL) {
        workers_ = MAX(static_cast<u32>(MAX(static_cast<s32>(thread::getNativeThreadCount()) - 1L, 0L)), 1uL);
    }

    /**
     * Prepare pipeline and workers
     */
    _schedule->setup();
    _workers = static_cast<aligned_worker*>(calloc(workers_, sizeof(aligned_worker)));

    /**
     * Construct every worker
     */
    u32 critCount { 0 };
    u32 ioCount { 0 };
    u32 allCount { workers_ };

    if (workers_ > 4) {
        critCount = static_cast<u32>(std::log2(workers_));
        allCount -= critCount;
    }

    if (workers_ > 8) {
        ioCount = static_cast<u32>(std::log(workers_) / std::log(8));
        ioCount = MIN(ioCount, (workers_ / 2) - 1);

        allCount -= ioCount;
    }

    /**
     * Construct workers with time critical task mask
     */
    u32 offset { 0 };
    for (u32 i = 0; i < critCount; ++i) {
        ::new(&_workers[offset + i]) aligned_worker(_schedule.get(), &_fiberPool, task::TaskMask::eCritical);
    }

    /**
     * Construct workers with io task mask
     */
    offset += critCount;
    for (u32 i = 0; i < ioCount; ++i) {
        ::new(&_workers[offset + i]) aligned_worker(_schedule.get(), &_fiberPool, task::TaskMask::eIO);
    }

    /**
     * Construct workers with all available masks
     */
    offset += ioCount;
    for (u32 i = 0; i < allCount; ++i) {
        ::new(&_workers[offset + i]) aligned_worker(_schedule.get(), &_fiberPool, task::TaskMask::eAll);
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

    auto tickPipeline = make_uptr<TickPipeline>();
    _pipeline->addPipeline(std::move(tickPipeline));

    /**/

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
        __debugbreak();
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

#include "Scheduler.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/stdafx.h>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace ember::engine::scheduler;
using namespace ember::engine;
using namespace ember;

ptr<Scheduler> Scheduler::_instance = nullptr;

Scheduler::Scheduler() noexcept :
    _pipeline(),
    _workerCount(0),
    _workers(nullptr),
    _fiberPool(0) {}

Scheduler::~Scheduler() {
    tidy();
}

Scheduler& Scheduler::get() {
    return *_instance;
}

ptr<Scheduler> Scheduler::get(_STD nothrow_t) noexcept {
    return _instance;
}

ptr<Scheduler> Scheduler::make() {
    if (_instance != nullptr) {
        return _instance;
    }

    _instance = make_ptr<Scheduler>();
    return _instance;
}

void Scheduler::destroy() noexcept {

    if (_instance == nullptr)
        return;

    delete _instance;
    _instance = nullptr;
}

#ifdef _DEBUG
void validate_task_ordering(task::__TaskDelegate task_) {
    const auto src { task_->srcStage() };
    const auto dst { task_->dstStage() };

    DEBUG_ASSERT(src.stage != ScheduleStage::eUndefined,
        "`ScheduleStage::eUndefined` is not allowed as src stage for task.")
    DEBUG_ASSERT(dst.stage != ScheduleStage::eAll,
        "`ScheduleStage::eAll` is not allowed as dst stage for task.")

    /**
     * Warning: Using io masked tasks with strong barrier guarantee can cause unexpected holding time for scheduler.
     * Warning: If io task blocks while having strong guarantee, the barrier release is enforced to wait for io task to complete.
     * Warning: IO tasks are most common not guaranteed to return immediatly, causing unpredictable latency for other tasks.
     */
    if (src.strong() && task_->mask() == task::TaskMask::eIO) {
        DEBUG_SNMSG(false, "WARN",
            "Scheduled task with strong barrier guarantee and `TaskMask::eIO`. Could block scheduler for unexpected time...")
    }

    /**
     * Warning: Using low(est) priority tasks with strong barrier guarantee will enforce scheduler to cycle at current stage barrier even if it's the last task
     * Warning: It might be common, that low(est) priority tasks will be at the bottom of execution queue for current barrier.
     * Warning: This might enforce serialized execution order, breaking parallel execution performance.
     */
    if (src.strong() && task_->mask() == task::TaskMask::eLower) {
        DEBUG_SNMSG(false, "WARN",
            "Scheduled task with strong barrier guarantee and `TaskMask::eLower`. Low(est) priority tasks should most likly be scheduled with weak guarantee...")
    }

    /**
     *
     */
    if (src.stage == dst.stage && src.weak() && dst.weak()) {
        DEBUG_SNMSG(false, "ERROR",
            "Scheduled task with weak barrier guarantee at src stage, while src stage, dst stage and current stage are the same, is not allowed and will cause undefined execution order for this task.")
    }
}
#endif

void waitReadyAll(u32 workerCount_, const ptr<const Scheduler::aligned_worker> workers_) {
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

void Scheduler::delay(task::__TaskDelegate task_, const u32 ticks_) {
    // TODO:
    #ifdef _DEBUG
    validate_task_ordering(task_);
    #endif
}

void Scheduler::exec(task::__TaskDelegate task_) {
    // TODO:
    #ifdef _DEBUG
    validate_task_ordering(task_);
    #endif
    _pipeline.push(_STD forward<task::__TaskDelegate>(task_));
}

size_t Scheduler::getWorkerCount() const {
    return _workerCount;
}

void Scheduler::tidy() {

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
}

void Scheduler::setup(u32 workers_) {

    SCOPED_STOPWATCH

    #ifdef USE_WORKER_COUNT
    workers_ = USE_WORKER_COUNT;
    #endif

    if (workers_ == 0ui32) {
        workers_ = MAX(static_cast<u32>(MAX(static_cast<s32>(thread::getNativeThreadCount()) - 1i32, 0i32)), 1ui32);
    }

    /**
     * Prepare pipeline and workers
     */
    _pipeline.setup(workers_);
    _workers = static_cast<aligned_worker*>(calloc(workers_, sizeof(aligned_worker)));

    /**
     * Construct every worker
     */
    u32 critCount { 0 };
    u32 ioCount { 0 };
    u32 allCount { workers_ };

    if (workers_ > 4) {
        critCount = static_cast<u32>(_STD log2(workers_));
        allCount -= critCount;
    }

    if (workers_ > 8) {
        ioCount = static_cast<u32>(_STD log(workers_) / _STD log(8));
        ioCount = MIN(ioCount, (workers_ / 2) - 1);

        allCount -= ioCount;
    }

    /**
     * Construct workers with time critical task mask
     */
    u32 offset { 0 };
    for (u32 i = 0; i < critCount; ++i) {
        ::new(&_workers[offset + i]) aligned_worker(&_pipeline, &_fiberPool, task::TaskMask::eCritical);
    }

    /**
     * Construct workers with io task mask
     */
    offset += critCount;
    for (u32 i = 0; i < ioCount; ++i) {
        ::new(&_workers[offset + i]) aligned_worker(&_pipeline, &_fiberPool, task::TaskMask::eIO);
    }

    /**
     * Construct workers with all available masks
     */
    offset += ioCount;
    for (u32 i = 0; i < allCount; ++i) {
        ::new(&_workers[offset + i]) aligned_worker(&_pipeline, &_fiberPool, task::TaskMask::eAll);
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

void Scheduler::wait() const {
    for (u32 i = 0; i < _workerCount; ++i) {
        #if _DEBUG
        assert(_workers[i].join());
        #else
        _workers[i].join();
        #endif
    }
}

#include "Scheduler.hpp"

#include <Engine.Common/Make.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace ember::engine::scheduler;
using namespace ember;

ptr<Scheduler> Scheduler::_instance = nullptr;

Scheduler::Scheduler() noexcept :
    _sharedTasks(),
    _workerCount(0),
    _workers(nullptr) {}

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

void Scheduler::delay(task::__TaskDelegate task_, const u32 ticks_) {
    // TODO:
}

void Scheduler::exec(task::__TaskDelegate task_) {
    // TODO:
    _sharedTasks.push(_STD forward<task::__TaskDelegate>(task_));
}

size_t Scheduler::getWorkerCount() const {
    return _workerCount;
}

void Scheduler::tidy() {

    SCOPED_STOPWATCH

    if (_workerCount) {
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

    if (workers_ == 0) {
        workers_ = MAX(thread::getNativeThreadCount() - 1, 1);
        // TODO:
        workers_ = MIN(workers_, 8);
    }

    /**
     * Prepare shared task collection
     */
    _sharedTasks.setup(workers_);
    _workers = static_cast<aligned_worker*>(malloc(sizeof(aligned_worker) * workers_));

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
        ::new(&_workers[offset + i]) aligned_worker(&_sharedTasks, task::TaskMask::eCritical);
    }

    /**
     * Construct workers with io task mask
     */
    offset += critCount;
    for (u32 i = 0; i < ioCount; ++i) {
        ::new(&_workers[offset + i]) aligned_worker(&_sharedTasks, task::TaskMask::eIO);
    }

    /**
     * Construct workers with all available masks
     */
    offset += ioCount;
    for (u32 i = 0; i < allCount; ++i) {
        ::new(&_workers[offset + i]) aligned_worker(&_sharedTasks, task::TaskMask::eAll);
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

#if 0

Scheduler::Scheduler() :
	_shared(),
	_threads(std::vector<types::Thread::__Thread>(0)) {
	_instance = this;
}

Scheduler::~Scheduler() {
	if (Scheduler::_instance == this) {
		Scheduler::_instance = nullptr;
	}

	for (const types::Thread::__Thread& entry : _threads)
		entry->interrupt();
	for (size_t i = 0; i < _threads.size(); i++)
		addTask([]() {});
	for (types::Thread::__Thread& entry : _threads)
		entry.reset();
}

void Scheduler::addTask(subroutine::Task&& task_) {
	_shared.enqueue(_STD move(task_));
}

void Scheduler::addTask(const subroutine::Task& task_) {
	_shared.enqueue(task_);
}

void Scheduler::addTask(const _STD function<void()>& fnc_) {
	_shared.enqueue(subroutine::make_task(fnc_));
}

void Scheduler::addTask(const batch_fnc_t& fnc_, uint32_t parallels) { }

void Scheduler::addCondTask(const std::function<bool()>& cnd_, const std::function<void()>& fnc_) {
	_shared.enqueue(subroutine::make_task(fnc_, cnd_));
}

void Scheduler::addCondTask(const batch_cnd_t& cond_, const batch_fnc_t& fnc_, uint32_t parallels_) { }

void Scheduler::addRepeatTask(const ::std::function<bool()>& cnd_, const ::std::function<void()>& fnc_) {
	_shared.enqueue(subroutine::make_task(fnc_, cnd_, subroutine::TaskType::eRepetitive));
}

void Scheduler::addRepeatTask(const batch_cnd_t& cnd_, const batch_fnc_t& fnc_, uint32_t parallels_) { }

size_t Scheduler::getWorkerCount() const {
	return _threads.size();
}

void Scheduler::setup(const size_t workers_) {
	/**
	 * Scale up threads to a certain level
	 */
	scale(workers_);
}

void Scheduler::wait() const {
	// TODO: make mutex lock for tasking
}

void Scheduler::scale(const size_t size_) {
	if (size_ < 1 || size_ == _threads.size())
		return;

	if (size_ > _threads.size()) {
		/**
		 * Reserve and allocate memory to get stable object references
		 */
		_shared.reserve(size_);
		_threads.reserve(size_);

		for (size_t i = 0; i < size_; i++) {
			/**
			 * Create new thread for thread pool and reference internal worker queue from shared queue.
			 */
			_threads.push_back(
				__internal::types::unique<types::Thread>(GetCurrentThreadId(), _shared.sharedFromPool())
			);
		}
	}

	if (size_ < _threads.size()) {
#if FALSE
		throw std::runtime_error("Can not shrink thread pool of tasking.");
#endif
		_Throw_system_error(_STD errc::operation_not_supported);
	}
}

#endif

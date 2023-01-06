#include "Thread.hpp"

#include <Engine.Common/__macro.hpp>
#include <Windows.h>
#include <processthreadsapi.h>
#include <stdexcept>

using namespace ember::engine::scheduler::thread;

FORCE_INLINE thread_id cast_ntid_tid(const std::thread::id& ntid_) {
    if constexpr (sizeof(ntid_) == sizeof(ember::u64)) {
        return *static_cast<const ember::u64*>(
            static_cast<const void*>(&ntid_)
        );
    } else {
        return *static_cast<const ember::u32*>(
            static_cast<const void*>(&ntid_)
        );
    }
}

// Warning: This function is called multiple times each round trip. Should be readable as fast as possible.
thread_local static thread_id __threadId { cast_ntid_tid(_STD this_thread::get_id()) };

[[nodiscard]] uint64_t generate_thread_idx() {
    static _STD atomic_uint64_t idx = { 0 };
    return idx.fetch_add(1, _STD memory_order_relaxed);
}

thread_local static uint64_t __threadIdx { generate_thread_idx() };

FORCE_INLINE bool set_priority(HANDLE handle_, priority priority_) {
    #if defined(_WIN32) || defined(_WIN64)
    auto tr = 0;

    switch (priority_) {
        case priority::eTimeCritical: {
            tr = THREAD_PRIORITY_TIME_CRITICAL;
            break;
        }
        case priority::eHigh: {
            tr = THREAD_PRIORITY_HIGHEST;
            break;
        }
        case priority::eNormal: {
            tr = THREAD_PRIORITY_NORMAL;
            break;
        }
        case priority::eLow: {
            tr = THREAD_PRIORITY_LOWEST;
            break;
        }
    }
    #endif

    return ::SetThreadPriority(handle_, tr) == TRUE;
}

Thread::Thread() :
    _handle(nullptr) {}

Thread::~Thread() {
    destroy();
}

bool Thread::start(const _STD function<void(void*)>& fnc_, void* args_) {
    if (_handle) {
        DEBUG_ASSERT(FALSE, "Thread has already valid 'handle_'")
        return false;
    }

    _handle = new _STD thread(fnc_, args_);

    DEBUG_ASSERT(_handle, "Failed to create valid 'handle_'")
    return _handle;
}

bool Thread::join() {
    DEBUG_ASSERT(_handle, "Thread has no valid 'handle_'")
    if (_handle->joinable()) {
        _handle->join();
        return true;
    }
    return false;
}

bool Thread::destroy() {
    delete _handle;
    _handle = nullptr;

    return true;
}

thread_id Thread::getId() const noexcept {
    DEBUG_ASSERT(_handle, "Thread has no valid 'handle_'")
    const auto ntid = _handle->get_id();
    return cast_ntid_tid(ntid);
}

affinity_mask Thread::setAffinity(affinity_mask mask_) {
    return ::SetThreadAffinityMask(_handle->native_handle(), mask_);
}

bool Thread::setPriority(priority priority_) {
    DEBUG_ASSERT(_handle, "Thread has no valid 'handle_'");
    return set_priority(_handle->native_handle(), priority_);
}

void Thread::setName(cstr name_) {
    throw std::runtime_error("unsupported operation.");
}

void self::yield() {
    _STD this_thread::yield();
}

void self::sleepFor(u64 milliseconds_) {
    _STD this_thread::sleep_for(_STD chrono::milliseconds(milliseconds_));
}

thread_id self::getId() noexcept {
    return __threadId;
}

ember::u64 self::getIdx() noexcept {
    return __threadIdx;
}

ember::u32 ember::engine::scheduler::thread::getNativeThreadCount() {
    return _STD thread::hardware_concurrency();
}

#if 0
#include "Subroutines/Task.hpp"

using namespace clockwork::__internal::scheduler::types;

Thread::Thread(DWORD parent_, TaskQueue& queue_) :
	_interrupt(false),
	_queue(queue_),
	_parentId(parent_) {
	_thread = _STD thread(&Thread::process, this);
}

Thread::~Thread() {
	if (_thread.joinable())
		_thread.join();
}

void Thread::process() {
	bool available = false;

#if TRUE
	MSG msg;
	PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

	AttachThreadInput(GetCurrentThreadId(), _parentId, TRUE);
#endif
	
	while (!_interrupt) {		
		const subroutine::__ptr__Task task = _queue.front();

		if (task == nullptr)
			continue;

		if (!task->check()) {
			_queue.defer(task);
		}

		(*task)();

		if (task->getType() == subroutine::eRepetitive) {
			_queue.defer(task);
		}
		
		_queue.pop();
	}
}

void Thread::interrupt() {
	_interrupt = true;
}
#endif

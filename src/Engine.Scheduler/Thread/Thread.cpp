#include "Thread.hpp"

#include <stdexcept>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Asserts/Todo.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/String/Convert.hpp>

#ifdef WIN32
#include <Engine.Common/stdafx.h>
/**/
#include <processthreadsapi.h>
#else
#include <pthread.h>
#endif

using namespace hg::engine::scheduler::thread;

FORCE_INLINE static inline thread_id cast_ntid_tid(const std::thread::id& ntid_) {
	if constexpr (sizeof(ntid_) == sizeof(hg::u64)) {
		return *static_cast<const hg::u64*>(
			static_cast<const void*>(&ntid_)
		);
	} else {
		return *static_cast<const hg::u32*>(
			static_cast<const void*>(&ntid_)
		);
	}
}

// Warning: This function is called multiple times each round trip. Should be readable as fast as possible.
thread_local static thread_id __threadId { cast_ntid_tid(std::this_thread::get_id()) };

[[nodiscard]] static uint64_t generate_thread_idx() {
	static std::atomic_uint64_t idx = { 0 };
	return idx.fetch_add(1, std::memory_order_relaxed);
}

thread_local static uint64_t __threadIdx { generate_thread_idx() };

FORCE_INLINE inline static bool set_priority(std::thread::native_handle_type handle_, priority priority_) {

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

	return ::SetThreadPriority(handle_, tr) == TRUE;

	#else

	int policy;
	sched_param params;
	::hg::assertrt(pthread_getschedparam(handle_, &policy, &params) == 0);

	const auto lowPriorityLimit = sched_get_priority_min(policy);
	const auto highPriorityLimit = sched_get_priority_max(policy);
	const auto priorityRange = lowPriorityLimit != highPriorityLimit ? (highPriorityLimit - lowPriorityLimit) : 0;
	const auto step = priorityRange / 4;

	switch (priority_) {
		case eLow: {
			return pthread_setschedprio(handle_, lowPriorityLimit + step * 0) == 0;
		}
		case eNormal: {
			return pthread_setschedprio(handle_, lowPriorityLimit + step * 1) == 0;
		}
		case eHigh: {
			return pthread_setschedprio(handle_, lowPriorityLimit + step * 2) == 0;
		}
		case eTimeCritical: {
			return pthread_setschedprio(handle_, lowPriorityLimit + step * 3) == 0;
		}
	}

	#endif
	::hg::todo_panic();
}

Thread::Thread() :
	_handle(nullptr) {}

Thread::~Thread() {
	destroy();
}

bool Thread::start(const std::function<void(void*)>& fnc_, void* args_) {
	if (_handle) {
		#ifdef _DEBUG
		/* "Thread has already valid 'handle_'" */
		::hg::panic();
		#endif
		return false;
	}

	_handle = new std::thread(fnc_, args_);

	::hg::assertd(_handle /* "Failed to create valid 'handle_'" */);
	return _handle;
}

bool Thread::join() {
	::hg::assertd(_handle /* "Thread has no valid 'handle_'" */);
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
	::hg::assertd(_handle /* "Thread has no valid 'handle_'" */);
	const auto ntid = _handle->get_id();
	return cast_ntid_tid(ntid);
}

affinity_mask Thread::setAffinity(affinity_mask mask_) {
	#ifdef WIN32
	return ::SetThreadAffinityMask(_handle->native_handle(), mask_);
	#endif
	::hg::todo_panic();
}

bool Thread::setPriority(priority priority_) {
	::hg::assertd(_handle /* "Thread has no valid 'handle_'" */);
	return set_priority(_handle->native_handle(), priority_);
}

void Thread::setName([[maybe_unused]] cstr name_) {
	#ifdef WIN32
	const auto next = stringToWString(name_);
	::hg::assertrt(SetThreadDescription(_handle->native_handle(), next.c_str()) == ERROR_SUCCESS);
	#else
	throw std::runtime_error("Unsupported operation.");
	#endif
}

void self::yield() {
	std::this_thread::yield();
}

void self::sleepFor(u64 milliseconds_) {
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds_));
}

thread_id self::getId() noexcept {
	return __threadId;
}

hg::u64 self::getIdx() noexcept {
	return __threadIdx;
}

hg::u32 hg::engine::scheduler::thread::getNativeThreadCount() {
	return std::thread::hardware_concurrency();
}

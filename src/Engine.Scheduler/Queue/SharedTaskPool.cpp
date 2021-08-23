#include "SharedTaskPool.hpp"

using namespace clockwork::__internal::scheduler;
using namespace clockwork::__internal::scheduler::types;

#if FALSE

SharedTaskPool::SharedTaskPool() :
	_mtx() { }

void SharedTaskPool::enqueue(subroutine::Task&& task_) {
	//__internal::types::SpinLock lck(_mtx);
	_STD unique_lock<_STD mutex> lck(_smtx);
	
	_pool.push(_STD move(task_));
}

void SharedTaskPool::enqueue(const subroutine::Task& task_) {
	//__internal::types::SpinLock lck(_mtx);
	_STD unique_lock<_STD mutex> lck(_smtx);
	
	_pool.push(task_);
}

TaskQueue& SharedTaskPool::sharedFromPool() {
	_threadQueues.push_back(TaskQueue(this));
	return _threadQueues[_threadQueues.size() - 1];
}

#endif
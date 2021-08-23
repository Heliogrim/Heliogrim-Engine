#include "TaskQueue.hpp"
#include "SharedTaskPool.hpp"

using namespace clockwork::__internal::scheduler;
using namespace clockwork::__internal::scheduler::types;

TaskQueue::TaskQueue() :
	_queue(),
	_shared(nullptr) { }

subroutine::__ptr__Task TaskQueue::front() {
	if (_queue.empty()) {
		_shared->request(_queue);
	}

	if (_queue.empty()) {
		return nullptr;
	}

	return &_queue.front();
}

void TaskQueue::defer(subroutine::__ptr__Task task_) {
	_shared->enqueue(*task_);
}

void TaskQueue::defer(const subroutine::Task& task_) {
	_shared->enqueue(task_);
}

void TaskQueue::pop() {
	_queue.pop_front();
}

void TaskQueue::push(subroutine::Task& task_) {
	_queue.push_back(task_);
}

TaskQueue::TaskQueue(__ptr__SharedTaskPool shared_) :
	_queue(nullptr),
	_shared(shared_) { }
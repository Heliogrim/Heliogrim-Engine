#pragma once
#include <Engine.Common/Concurrent/SpinLock.hpp>
#include "TaskPool.hpp"
#include "TaskQueue.hpp"
#include "../Subroutines/Task.hpp"

namespace clockwork::__internal::scheduler::types {
	/**
	 * \brief The task pool which covers all internal task pools of threads.\n
	 *	It will pull the next task pool from the queue if the current might be empty.
	 */
	class SharedTaskPool {
		friend TaskQueue;

	public:
		/**
		 * \brief 
		 */
		SharedTaskPool();

		/**
		 * \brief 
		 */
		~SharedTaskPool() {}

		/**
		 * \brief 
		 * \param task_ 
		 */
		void enqueue(subroutine::Task&& task_);

		/**
		 * \brief 
		 * \param task_ 
		 */
		void enqueue(const subroutine::Task& task_);

		/**
		 * \brief 
		 * \return 
		 */
		TaskQueue& sharedFromPool();

		/**
		 * \brief 
		 * \param size_ 
		 */
		void reserve(const size_t size_) {
			if (_threadQueues.capacity() < size_) {
				_threadQueues.reserve(size_);
			}
		}

		#pragma message("This method is currently absolutly unsafe: " __FILE__)
		#pragma warning(disable : 4996)
		[[deprecated("This method should not be used.")]] void each(const _STD function<void()>& fnc_) {
			lock();
			for (auto& entry : _threadQueues) {
				subroutine::Task task { fnc_ };
				entry.push(task);
			}
			unlock();
		}

	protected:
		/**
		 * \brief
		 * TODO: The whole operation should be atomic or atomic like which it isn't obviously.\
		 *	We need to guarantee progress for the requesting thread \
		 *	Locking the whole operation like it's temporary done, will cause huge decrease of performance while cycle over time
		 */
		void request(TaskQueue::queue_type& queue_) {
			/* TODO: Temporary lock */
			//__internal::types::SpinLock lck(_mtx);
			_STD unique_lock<_STD mutex> lck(_smtx);

			// TODO: check for left tasks at current pool
			/**
			 * TODO: cause pop and push of queue is not trivial and not atomic like, a SpinLock might be ineffective
			 * TaskPool should be able to lock a sequence of tasks with a certain number of atomic_flag / atomic_bool to test_lock
			 * If the test lock has a negative result, we need to step to the next task source
			 */
			subroutine::Task tmp;
			/* while */
			if (_pool.pop(tmp)) {
				queue_.push_back(_STD move(tmp));
			}

			if (!queue_.empty()) {
				return;
			}

			// TODO: check for worksteeling from other threads
			// TODO: We need to determine the queue size for each thread and check whether it is useful to steel some tasks\n If so, we will split the work queue into two sequences without touching the lower sequence which might be currently processed\n The upper sequence should be moved to the current steeling threads internal queue\n The whole process needs to be locked, so no third party ( thread ) will effect the steeling process
			/**
			 * 
			 */
		}

		/**
		 * \brief 
		 */
		[[deprecated("This method should not be used.")]] void lock() {
			_smtx.lock();
		}

		/**
		 * \brief 
		 */
		[[deprecated("This method should not be used.")]] void unlock() {
			_smtx.unlock();
		}

	private:
		/**
		 * \brief 
		 */
		_STD vector<TaskQueue> _threadQueues;

		/**
		 * \brief 
		 */
		TaskPool _pool;

		/**
		 * \brief 
		 */
		mutable __internal::types::SpinMutex _mtx;

		mutable _STD mutex _smtx;
	};
}

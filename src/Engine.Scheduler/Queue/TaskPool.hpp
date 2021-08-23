#pragma once
#include <deque>
#include "../Subroutines/Task.hpp"

namespace clockwork::__internal::scheduler::types {
	/**
	 * \brief
	 */
	class TaskPool {		
	public:
		/**
		 * \brief 
		 */
		~TaskPool() { }
		
		/**
		 * \brief 
		 * \param task_ Reference for return value is success
		 * \return true if popped successfully
		 */
		bool pop(subroutine::Task& task_) {
			if (_queue.empty()) {
				return false;
			}

			task_ = _STD move(_queue.front());
			_queue.pop_front();
			return true;
		}

		/**
		 * \brief
		 * \param fnc_ 
		 */
		void push(subroutine::Task&& fnc_) {
			_queue.push_back(_STD move(fnc_));
		}

		/**
		 * \brief 
		 * \param task_ 
		 */
		void push(const subroutine::Task& task_) {
			_queue.push_back(task_);
		}

		/**
		 * \brief 
		 * \param left_ 
		 * \param right_ 
		 */
		static void swap(TaskPool& left_, TaskPool& right_) noexcept {
			left_._queue.swap(right_._queue);
		}

		/**
		 * \brief
		 * \param other_ 
		 */
		void swap(TaskPool& other_) noexcept {
			_queue.swap(other_._queue);
		}
		
	private:
		/**
		 * \brief 
		 */
		_STD deque<subroutine::Task> _queue;
	};
}
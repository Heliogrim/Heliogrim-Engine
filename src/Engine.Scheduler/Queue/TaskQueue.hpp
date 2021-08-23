#pragma once
#include <deque>
#include "../Subroutines/Task.hpp"

namespace clockwork::__internal::scheduler {
	class SharedTaskPool;
	using __ptr__SharedTaskPool = SharedTaskPool*;

#ifdef ENVx64
	template <typename Ty_, size_t cSize_ = 64>
#else
	template<typename Ty_, size_t cSize_ = 32>
#endif
	class pseudo_queue :
		public _STD vector<Ty_> {
	public:
		static constexpr const size_t capacity_d = cSize_;
		
		using size_type = typename _STD vector<Ty_>::size_type;

		using iterator = typename _STD vector<Ty_>::iterator;
		using const_iterator = typename _STD vector<Ty_>::const_iterator;
		using reference = typename _STD vector<Ty_>::reference;
		using pointer = typename _STD vector<Ty_>::pointer;

		/**
		 * \brief 
		 */
		pseudo_queue() :
			_STD vector<Ty_>(),
			_front(0) { }

		/**
		 * \brief 
		 */
		explicit pseudo_queue(nullptr_t) :
			_STD vector<Ty_>(),
			_front(0) {
			_STD vector<Ty_>::reserve(cSize_);
		}

		/**
		 * \brief 
		 */
		pseudo_queue(const pseudo_queue<Ty_, cSize_>& other_) :
			_STD vector<Ty_>::vector(other_),
			_front(other_._front) {
			_STD vector<Ty_>::reserve(other_.capacity());
		}

		/**
		 * \brief 
		 */
		pseudo_queue(pseudo_queue<Ty_, cSize_>&& other_) noexcept :
			_STD vector<Ty_>::vector(_STD move(other_)),
			_front(_STD exchange(other_._front, 0)) { }

		/**
		 * \brief 
		 */
		~pseudo_queue() {
			if (_front > 0) {
				const auto _Result = _STD vector<Ty_>::begin();
				_STD fill(_Result, _Result + _front, Ty_());
			}
			
			static_cast<_STD vector<Ty_>*>(this)->~vector();
		}

		/**
		 * \brief 
		 * \return 
		 */
		reference front() {
			return _STD vector<Ty_>::operator[](_front);
		}

		/**
		 * \brief 
		 */
		void pop_front() {
			/*
			// Erase elements at front position
			pointer _Data = _STD vector<Ty_>::data();
			_STD vector<Ty_>::get_allocator().destroy(_Data + (++_front));
			 */
			if ((++_front) >= _STD vector<Ty_>::size()) {
				_STD vector<Ty_>::clear();
				_front = 0;
			}
		}

		/**
		 * \brief 
		 * \return 
		 */
		size_type front_left() const {
			return _STD vector<Ty_>::size() - _front;
		}

		/**
		 * \brief 
		 */
		void clear() noexcept = delete;

		/**
		 * \brief 
		 * \return 
		 */
		iterator erase(const_iterator) = delete;

		/**
		 * \brief
		 * \return 
		 */
		iterator erase(const_iterator, const_iterator) = delete;

		/**
		 * \brief 
		 * \param other_ 
		 * \return 
		 */
		pseudo_queue<Ty_, cSize_>& operator=(const pseudo_queue<Ty_, cSize_>& other_) {
			_STD vector<Ty_>::operator=(other_);
			if (this != _STD addressof(other_)) {
				_front = other_._front;
			}
			
			return *this;
		}

		/**
		 * \brief 
		 * \param other_ 
		 * \return 
		 */
		pseudo_queue<Ty_, cSize_>& operator=(pseudo_queue<Ty_, cSize_>&& other_) noexcept {
			_STD vector<Ty_>::operator=(_STD move(other_));
			if (this != _STD addressof(other_)) {
				_front = _STD exchange(other_._front, 0);
			}
			
			return *this;
		}

		/**
		 * \brief 
		 * \param other_ 
		 */
		void swap(pseudo_queue<Ty_, cSize_>& other_) noexcept {
			if (this != _STD addressof(other_)) {
				_STD vector<Ty_>::swap(other_);
				_STD swap(_front, other_._front);
			}
		}

	private:
		/**
		 * \brief 
		 */
		size_type _front;
	};

	/**
	 * \brief 
	 * \tparam Ty_ 
	 * \tparam cSize_ 
	 * \param left_ 
	 * \param right_ 
	 */
	template <class Ty_, size_t cSize_ = pseudo_queue<Ty_>::_capacity_d>
	void swap(pseudo_queue<Ty_, cSize_>& left_, pseudo_queue<Ty_, cSize_>& right_) noexcept {
		left_.swap(right_);
	}

	/**
	 * \brief This is the actual task pool for a single thread. It holds the current tasks to execute at current state.\n
	 *	If the task pool is empty, it will callback for new tasks.
	 */
	class TaskQueue {
		friend SharedTaskPool;

	public:
		/**
		 * \brief 
		 */
		using queue_type = pseudo_queue<subroutine::Task>;

		/**
		 * \brief 
		 */
		TaskQueue();

		/**
		 * \brief
		 * \param other_ 
		 */
		TaskQueue(const TaskQueue& other_) :
			_queue(other_._queue),
			_shared(other_._shared) { }

		/**
		 * \brief 
		 * \param other_ 
		 */
		TaskQueue(TaskQueue&& other_) noexcept :
			_queue(_STD move(other_._queue)),
			_shared(_STD move(other_._shared)) { }

		/**
		 * \brief 
		 * \return 
		 */
		subroutine::Task* front();

		/**
		 * \brief 
		 * \param task_ 
		 */
		void defer(subroutine::__ptr__Task task_);

		/**
		 * \brief 
		 * \param task_ 
		 */
		void defer(const subroutine::Task& task_);

		/**
		 * \brief 
		 * \return 
		 */
		void pop();

		/**
		 * \brief 
		 * \param task_
		 */
		void push(subroutine::Task& task_);

	protected:

		/**
		 * \brief 
		 * \param shared_
		 */
		explicit TaskQueue(__ptr__SharedTaskPool shared_);

	private:
		/**
		 * \brief 
		 */
		queue_type _queue;

		/**
		 * \brief
		 * // TODO: check whether reference or __ptr__SharedTaskPool should be used
		 */
		__ptr__SharedTaskPool _shared;
	};
}

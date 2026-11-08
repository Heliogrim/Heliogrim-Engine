#pragma once

#include <atomic>
#include <utility>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::scheduler::fiber {
	struct Fiber;
}

namespace hg::engine::scheduler {
	struct ParkNode {
		enum class State : u8 {
			eIdle,
			eArmed,
			eParked,
			eWoken
		};

		std::atomic<State> state { State::eIdle };
		ptr<ParkNode> next = nullptr;
		ptr<fiber::Fiber> fiber = nullptr;
		std::atomic<u32> threadFlag { 0 };
	};

	class ParkQueue {
	public:
		/**
		 * View handed to locked sections; operates on the list without re-taking the lock.
		 */
		class Locked {
		private:
			friend class ParkQueue;

		private:
			explicit Locked(ref<ParkQueue> queue_) noexcept :
				_queue(queue_) {}

		private:
			ref<ParkQueue> _queue;

		public:
			void push(ref<ParkNode> node_) noexcept {
				_queue.pushUnlocked(node_);
			}

			[[nodiscard]] ptr<ParkNode> popOne() noexcept {
				return _queue.popOneUnlocked();
			}

			[[nodiscard]] ptr<ParkNode> takeAll() noexcept {
				return _queue.takeAllUnlocked();
			}

			[[nodiscard]] bool empty() const noexcept {
				return _queue._head == nullptr;
			}
		};

	private:
		mutable std::atomic_flag _lck {};
		ptr<ParkNode> _head = nullptr;
		ptr<ParkNode> _tail = nullptr;

	public:
		void push(ref<ParkNode> node_) noexcept {
			lock();
			pushUnlocked(node_);
			unlock();
		}

		[[nodiscard]] ptr<ParkNode> popOne() noexcept {
			lock();
			auto* const node = popOneUnlocked();
			unlock();
			return node;
		}

		[[nodiscard]] ptr<ParkNode> takeAll() noexcept {
			lock();
			auto* const node = takeAllUnlocked();
			unlock();
			return node;
		}

		[[nodiscard]] bool empty() const noexcept {
			lock();
			const bool res = _head == nullptr;
			unlock();
			return res;
		}

		/**
		 * Runs fn_ under the queue lock; used by primitives to re-check a predicate before pushing.
		 */
		template <typename Fn_>
		decltype(auto) locked(Fn_&& fn_) {
			struct Guard {
				ref<const ParkQueue> self;

				~Guard() {
					self.unlock();
				}
			} guard { *this };
			lock();
			return std::forward<Fn_>(fn_)(Locked { *this });
		}

	private:
		void pushUnlocked(ref<ParkNode> node_) noexcept {
			node_.next = nullptr;
			if (_tail != nullptr) {
				_tail->next = &node_;
			} else {
				_head = &node_;
			}
			_tail = &node_;
		}

		[[nodiscard]] ptr<ParkNode> popOneUnlocked() noexcept {
			auto* const node = _head;
			if (node != nullptr) {
				_head = node->next;
				if (_head == nullptr) {
					_tail = nullptr;
				}
				node->next = nullptr;
			}
			return node;
		}

		[[nodiscard]] ptr<ParkNode> takeAllUnlocked() noexcept {
			auto* const node = _head;
			_head = nullptr;
			_tail = nullptr;
			return node;
		}

		void lock() const noexcept {
			while (_lck.test_and_set(std::memory_order::acquire)) {}
		}

		void unlock() const noexcept {
			_lck.clear(std::memory_order::release);
		}
	};
}

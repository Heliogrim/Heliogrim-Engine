#pragma once

#include <atomic>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::scheduler::fiber {
	class Fiber;
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
		void push(ref<ParkNode> node_) noexcept {
			lock();
			node_.next = nullptr;
			if (_tail != nullptr) {
				_tail->next = &node_;
			} else {
				_head = &node_;
			}
			_tail = &node_;
			unlock();
		}

		[[nodiscard]] ptr<ParkNode> popOne() noexcept {
			lock();
			auto* const node = _head;
			if (node != nullptr) {
				_head = node->next;
				if (_head == nullptr) {
					_tail = nullptr;
				}
				node->next = nullptr;
			}
			unlock();
			return node;
		}

		[[nodiscard]] ptr<ParkNode> takeAll() noexcept {
			lock();
			auto* const node = _head;
			_head = nullptr;
			_tail = nullptr;
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
			return std::forward<Fn_>(fn_)();
		}

	private:
		void lock() const noexcept {
			while (_lck.test_and_set(std::memory_order::acquire)) {}
		}

		void unlock() const noexcept {
			_lck.clear(std::memory_order::release);
		}

	private:
		mutable std::atomic_flag _lck {};
		ptr<ParkNode> _head = nullptr;
		ptr<ParkNode> _tail = nullptr;
	};
}

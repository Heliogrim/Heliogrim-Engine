#pragma once

#include <condition_variable>
#include <Engine.Common/Functional/UniqueFn.hpp>
#include <Engine.Async/Collection/RingBuffer.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::scheduler::task {
	class SignaledQueue final {
	public:
		using value_type = SignaledQueue;
		using reference_type = ref<value_type>;
		using const_reference_type = cref<value_type>;

	public:
		SignaledQueue() = default;

		~SignaledQueue() = default;

	public:
		void signal();

		void finalize();

		[[nodiscard]] bool finalized() const noexcept;

		[[nodiscard]] bool empty() const noexcept;

		bool try_push(mref<hg::UniqueFn<void(), 128>> fnc_);

	private:
		bool pop(OUT ref<hg::UniqueFn<void(), 128>> fnc_);

	public:
		bool waitPop(OUT ref<hg::UniqueFn<void(), 128>> fnc_);

	private:
		std::mutex _mtx;
		std::condition_variable _sig;
		std::atomic_flag _mode;

	private:
		concurrent::RingBuffer<hg::UniqueFn<void(), 128>> _buffer;
	};
}

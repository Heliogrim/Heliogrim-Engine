#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Concurrent/Collection/RingBuffer.hpp>

#include "../Task/Task.hpp"

namespace hg::engine::scheduler {
	/**
	 * Forward Declaration
	 */
	class SharedBufferPool;

	class PooledBuffer :
		protected concurrent::RingBuffer<non_owning_rptr<const task::TaskDelegate>> {
	public:
		using underlying_type = concurrent::RingBuffer<non_owning_rptr<const task::TaskDelegate>>;

	public:
		PooledBuffer(_In_ const ptr<SharedBufferPool> pool_) noexcept;

		PooledBuffer(_In_ const ptr<SharedBufferPool> pool_, _In_ const underlying_type::size_type reserved_);

		~PooledBuffer();

	private:
		ptr<SharedBufferPool> _pool;

	public:
		/**
		 * Release this buffer back to pool
		 *
		 * @author Julius
		 * @date 17.11.2021
		 */
		void release();

	private:
		std::atomic_flag _writer;
		std::atomic_flag _reader;

	public:
		[[nodiscard]] bool try_pop(_Out_ ref<non_owning_rptr<const task::TaskDelegate>> value_);

		[[nodiscard]] bool try_push(_In_ mref<non_owning_rptr<const task::TaskDelegate>> value_);

		[[nodiscard]] bool empty() const noexcept;

		[[nodiscard]] bool full() const noexcept;
	};
}

#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "PooledBuffer.hpp"
#include "../Task/Task.hpp"

namespace hg::engine::scheduler {
	class SharedBufferPool {
	public:
		using aligned_buffer = ALIGNED(PooledBuffer, CACHE_LINE_SIZE);

		using size_type = uint_fast16_t;
		using atomic_size_type = std::atomic_uint_fast16_t;
		using capacity_type = uint_fast16_t;

	public:
		SharedBufferPool();

		~SharedBufferPool();

	private:
		void tidy();

	private:
		std::atomic<ptr<ptr<aligned_buffer>>> _pooled;
		atomic_size_type _poolSize;
		capacity_type _poolCapacity;

	public:
		[[nodiscard]] cref<atomic_size_type> size() const noexcept;

		[[nodiscard]] capacity_type capacity() const noexcept;

	public:
		[[nodiscard]] _Success_(return != nullptr) ptr<aligned_buffer> acquire();

		void release(_Inout_ mref<ptr<aligned_buffer>> buffer_);

	private:
		std::atomic_flag _blocked;

	private:
		/**
		 * Will grow the pooled buffers, but have to be synchronized externally
		 *
		 * @author Julius
		 * @date 12.11.2021
		 *
		 * @param capacity_ The capacity this pool should grow to.
		 */
		void grow(_In_ const capacity_type capacity_);

	public:
		/**
		 * Will reserve at least the given amount of buffers
		 *
		 * @author Julius
		 * @date 12.11.2021
		 *
		 * @param size_ The minimal size this pool should have.
		 *
		 * @returns True if successfully grown, otherwise false.
		 */
		_Success_(return == true) bool reserve(_In_ const size_type size_);

		/**
		 * Warning: Resizing of concurrent accessed pool is not permitted or supporter
		 *  This pool can only grow while being used
		 *
		 * @author Julius
		 * @date 12.11.2021
		 *
		 * @param size_ The size this pool should resize to.
		 *
		 * @returns True is successfully changed size, otherwise false.
		 */
		_Success_(return == true) bool resize(_In_ const size_type size_) = delete;
	};
}

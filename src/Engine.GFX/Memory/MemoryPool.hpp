#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "MemoryLayout.hpp"
#include "__fwd.hpp"

namespace hg::engine::gfx::memory {
	class MemoryPool {
	public:
		using this_type = MemoryPool;

	public:
		MemoryPool(cref<MemoryLayout> layout_);

		MemoryPool (cref<this_type>) = delete;

		MemoryPool (mref<this_type>) = delete;

		~MemoryPool();

	public:
		void tidy();

	private:
		MemoryLayout _layout;

	public:
		[[nodiscard]] cref<MemoryLayout> layout() const noexcept;

	private:
		DenseMap<ptr<AllocatedMemory>, uptr<AllocatedMemory>> _pooling;
		Vector<uptr<AllocatedMemory>> _memory;

		u64 _totalMemory;
		std::atomic_uint_fast64_t _totalUsedMemory;
		std::atomic_uint_fast32_t _totalAlloc;

	public:
		[[nodiscard]] u64 totalMemory() const noexcept;

		[[nodiscard]] u64 usedMemory() const noexcept;

		[[nodiscard]] u64 freeMemory() const noexcept;

		[[nodiscard]] u32 allocCount() const noexcept;

		[[nodiscard]] u32 freeAllocCount() const noexcept;

	public:
		void push(mref<uptr<AllocatedMemory>> memory_);

	private:
		[[nodiscard]] bool treeMerge(
			cref<decltype(_memory)::iterator> begin_,
			cref<decltype(_memory)::iterator> end_,
			_Inout_ ref<uptr<AllocatedMemory>> memory_
		);

		void treeSplice(mref<uptr<AllocatedMemory>> memory_, const u64 targetSize_);

		void storeSubAllocate(mref<uptr<AllocatedMemory>> memory_);

		void releaseSubAllocation(cref<uptr<AllocatedMemory>> memory_) const;

		[[nodiscard]] AllocationResult allocate(
			const u64 size_,
			const bool bestFit_,
			_Out_ ref<uptr<AllocatedMemory>> dst_
		);

		bool free(_Inout_ ref<uptr<AllocatedMemory>> mem_, bool cascade_);

	public:
		[[nodiscard]] AllocationResult allocate(const u64 size_, _Out_ ref<uptr<AllocatedMemory>> dst_);

		bool free(_Inout_ ref<uptr<AllocatedMemory>> mem_);

	private:
		[[nodiscard]] bool poolContains(nmpt<AllocatedMemory> allocated_) const noexcept;
	};
}
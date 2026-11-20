#pragma once

#include <Engine.Common/Sal.hpp>

#include "MemoryLayout.hpp"
#include "__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics::memory {
	class MemoryCache {
	public:
		using this_type = MemoryCache;

	public:
		MemoryCache() noexcept;

		~MemoryCache();

	public:
		void tidy();

	private:
		DenseMap<MemoryLayout, ptr<MemoryPool>> _pools;

	public:
		[[nodiscard]] non_owning_rptr<MemoryPool> getOrCreatePool(cref<MemoryLayout> layout_);

		[[nodiscard]] AllocationResult allocate(
			cref<MemoryLayout> layout_,
			const u64 size_,
			_Out_ ref<uptr<AllocatedMemory>> dst_
		);

		bool free(_Inout_ ref<uptr<AllocatedMemory>> mem_);
	};
}

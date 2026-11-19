#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/__macro.hpp>

#include "__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics::memory {
	class macro_novtable Allocator {
	public:
		using this_type = Allocator;

	protected:
		Allocator() noexcept = default;

	public:
		virtual ~Allocator() noexcept = default;

	public:
		[[nodiscard]] virtual AllocationResult allocate(
			cref<MemoryLayout> layout_,
			const u64 size_,
			_Out_ ref<uptr<AllocatedMemory>> dst_
		) = 0;

		virtual void free(mref<uptr<AllocatedMemory>> mem_) = 0;
	};
}

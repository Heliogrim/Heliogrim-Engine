#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::acs {
	/**
	 * Strided view of one component across the slots of a chunk; `base` points at the component in the first slot.
	 */
	template <typename Type_>
	struct ComponentSpan {
		ptr<std::byte> base;
		u32 count;
		u32 stride;

		[[nodiscard]] Type_& operator[](const u32 index_) const noexcept {
			return *std::launder(reinterpret_cast<ptr<Type_>>(base + static_cast<size_t>(index_) * stride));
		}
	};
}

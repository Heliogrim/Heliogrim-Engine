#pragma once

#include "../vkinc.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
	/**
	 * Index of the first device memory type allowed by types_ that has all of flags_; throws if none does.
	 */
	[[nodiscard]] uint32_t get_memory_type(
		ref<const vk::PhysicalDevice> device_,
		const uint32_t types_,
		ref<const vk::MemoryPropertyFlags> flags_
	);
}

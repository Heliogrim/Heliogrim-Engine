#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "../vkinc.hpp"

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

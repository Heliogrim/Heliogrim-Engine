#pragma once

#include "../vkinc.hpp"

namespace hg::engine::graphics {
	/**
	 * Index of the first device memory type allowed by types_ that has all of flags_; throws if none does.
	 */
	[[nodiscard]] uint32_t get_memory_type(
		const vk::PhysicalDevice& device_,
		const uint32_t types_,
		const vk::MemoryPropertyFlags& flags_
	);
}

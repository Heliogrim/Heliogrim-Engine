#include "MemoryType.hpp"

#include <stdexcept>

using namespace hg::engine::graphics;
using namespace hg;

uint32_t engine::graphics::get_memory_type(
	ref<const vk::PhysicalDevice> device_,
	const uint32_t types_,
	ref<const vk::MemoryPropertyFlags> flags_
) {
	const vk::PhysicalDeviceMemoryProperties memProps = device_.getMemoryProperties();

	for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
		if ((types_ & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & flags_) == flags_)
			return i;
	}

	throw std::runtime_error("Failed to find suitable memory type.");
}

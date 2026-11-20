#pragma once

#include <Engine.Graphics/API/__vkFwd.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::driver::vk {
	struct VkDescriptorPoolSize {
		::vk::DescriptorType type;
		u32 capacity;
	};

	class VkDescriptorPool {
	public:
		using this_type = VkDescriptorPool;

	public:
		VkDescriptorPool() noexcept;

		VkDescriptorPool(cref<this_type>) = delete;

		VkDescriptorPool(mref<this_type> other_) noexcept;

		~VkDescriptorPool() noexcept;

	public:
		ref<this_type> operator=(mref<this_type>) noexcept = default;

		ref<this_type> operator=(cref<this_type>) = delete;

	public:
		Vector<VkDescriptorPoolSize> pooled;
		::VkDescriptorPool vkPool;

	public:
		void reset() noexcept;
	};

	/**/

	struct VkDescriptorPoolAllocationLayout {
		u32 maxSets;
		Vector<VkDescriptorPoolSize> sizes;
	};
}

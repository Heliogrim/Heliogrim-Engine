#pragma once

#include <Engine.GFX/API/__vkFwd.hpp>

#include "ComputePipeline.hpp"

namespace hg::engine::accel {
	class VkComputePipeline final :
		public InheritMeta<VkComputePipeline, ComputePipeline> {
	public:
		using this_type = VkComputePipeline;

	public:
		~VkComputePipeline() override = default;

	private:
		void tidy();

	private:
	public:
		_::VkComputePipelineLayout _vkPipeLayout;
		_::VkComputePipeline _vkPipe;

	public:
		void setVkPipeLayout(mref<_::VkComputePipelineLayout> vkPipeLayout_);

		void setVkPipe(mref<_::VkComputePipeline> vkPipe_);

		// Warning: Temporary
	public:
		Vector<_::VkDescriptorSetLayout> _vkDescLayouts;
	};
}

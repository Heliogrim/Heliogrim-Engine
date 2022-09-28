#include "VkComputePipeline.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace ember::engine::gfx;
using namespace ember;

VkComputePipeline::VkComputePipeline(sptr<Device> device_) :
    ComputePipeline(),
    _device(device_) {}

VkComputePipeline::~VkComputePipeline() noexcept = default;

void VkComputePipeline::setup() {

    SCOPED_STOPWATCH

    assert(_device);

    /**
     * Prepare
     */
    vk::ComputePipelineCreateInfo info {};

    /**
     *
     */
    Vector<vk::DescriptorSetLayout> descriptorLayouts {};

    info.stage = _shaderSlot.shader(true)->vkShaderStageInfo();

    for (const auto& entry : _shaderSlot.shader()->bindings()) {

        const auto& layout { entry.vkSetLayout() };

        if (_STD find(descriptorLayouts.begin(), descriptorLayouts.end(), layout) != descriptorLayouts.end()) {
            continue;
        }

        descriptorLayouts.push_back(layout);
    }

    /**
     * Pipeline Layout
     */
    const vk::PipelineLayoutCreateInfo layoutInfo {
        vk::PipelineLayoutCreateFlags(),
        static_cast<u32>(descriptorLayouts.size()),
        descriptorLayouts.data(),
        0,
        nullptr
    };

    _vkLayout = _device->vkDevice().createPipelineLayout(layoutInfo);
    assert(_vkLayout);

    /**
     *
     */
    info.basePipelineHandle = nullptr;
    info.basePipelineIndex = -1i32;

    info.layout = _vkLayout;

    const auto result = _device->vkDevice().createComputePipeline({}, info);
    assert(result.result == vk::Result::eSuccess);
    _vkPipeline = result.value;
}

void VkComputePipeline::destroy() noexcept {

    SCOPED_STOPWATCH

    if (_vkPipeline) {
        _device->vkDevice().destroyPipeline(_vkPipeline);
        _vkPipeline = nullptr;
    }

    if (_vkLayout) {
        _device->vkDevice().destroyPipelineLayout(_vkLayout);
        _vkLayout = nullptr;
    }

}

cref<vk::PipelineLayout> VkComputePipeline::vkLayout() const noexcept {
    return _vkLayout;
}

cref<vk::Pipeline> VkComputePipeline::vkPipeline() const noexcept {
    return _vkPipeline;
}

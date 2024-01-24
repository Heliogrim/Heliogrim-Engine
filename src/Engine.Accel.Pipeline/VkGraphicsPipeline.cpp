#include "VkGraphicsPipeline.hpp"

#include <stdexcept>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>

using namespace hg::engine::accel;
using namespace hg;

VkGraphicsPipeline::VkGraphicsPipeline(mref<smr<const AccelerationEffect>> effect_) :
    InheritMeta(std::move(effect_)),
    _vkPipeLayout(nullptr),
    _vkPipe(nullptr) {}

VkGraphicsPipeline::~VkGraphicsPipeline() {
    tidy();
}

void VkGraphicsPipeline::tidy() {

    const auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();

    if (_vkPipe) {
        device->vkDevice().destroyPipeline(reinterpret_cast<VkPipeline>(_vkPipe));
        _vkPipe = nullptr;
    }

    if (_vkPipeLayout) {
        device->vkDevice().destroyPipelineLayout(reinterpret_cast<VkPipelineLayout>(_vkPipeLayout));
        _vkPipeLayout = nullptr;
    }

    if (not _vkDescLayouts.empty()) {
        for (auto&& layout : _vkDescLayouts) {
            device->vkDevice().destroyDescriptorSetLayout(reinterpret_cast<VkDescriptorSetLayout>(layout));
        }
        _vkDescLayouts.clear();
    }

}

void VkGraphicsPipeline::setStageDerivat(const size_t idx_, mref<smr<StageDerivat>> derivat_) {

    if (idx_ > _stages.size()) {
        throw std::runtime_error("");
    }

    auto where = _stages.begin() + idx_;
    _stages.insert(where, std::move(derivat_));
}

void VkGraphicsPipeline::setVkPipeLayout(mref<_::VkGraphicsPipelineLayout> vkPipeLayout_) {
    _vkPipeLayout = std::move(vkPipeLayout_);
}

void VkGraphicsPipeline::setVkPipe(mref<_::VkGraphicsPipeline> vkPipe_) {
    _vkPipe = std::move(vkPipe_);
}

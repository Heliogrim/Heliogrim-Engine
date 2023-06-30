#include "VkAccelerationGraphicsPass.hpp"

#include <stdexcept>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>

using namespace hg::engine::gfx::acc;
using namespace hg;

VkAccelerationGraphicsPass::VkAccelerationGraphicsPass(mref<smr<AccelerationEffect>> effect_) :
    AccelerationGraphicsPass(_STD move(effect_)),
    _vkPipeLayout(nullptr),
    _vkPipe(nullptr) {}

VkAccelerationGraphicsPass::~VkAccelerationGraphicsPass() {
    tidy();
}

void VkAccelerationGraphicsPass::tidy() {

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

void VkAccelerationGraphicsPass::setStageDerivat(const size_t idx_, mref<smr<AccelerationStageDerivat>> derivat_) {

    if (idx_ > _stages.size()) {
        throw _STD runtime_error("");
    }

    auto where = _stages.begin() + idx_;
    _stages.insert(where, _STD move(derivat_));
}

void VkAccelerationGraphicsPass::setVkPipeLayout(mref<_::VkGraphicsPipelineLayout> vkPipeLayout_) {
    _vkPipeLayout = _STD move(vkPipeLayout_);
}

void VkAccelerationGraphicsPass::setVkPipe(mref<_::VkGraphicsPipeline> vkPipe_) {
    _vkPipe = _STD move(vkPipe_);
}

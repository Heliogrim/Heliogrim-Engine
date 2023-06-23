#include "VkAccelerationGraphicsPass.hpp"

#include <stdexcept>

using namespace hg::engine::gfx::acc;
using namespace hg;

VkAccelerationGraphicsPass::VkAccelerationGraphicsPass(mref<smr<AccelerationEffect>> effect_) :
    AccelerationGraphicsPass(_STD move(effect_)),
    _vkPipeLayout(nullptr),
    _vkPipe(nullptr) {}

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

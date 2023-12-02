#include "VkGraphicsPass.hpp"

#include <Engine.Accel.Pipeline/VkGraphicsPipeline.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Reflect/Cast.hpp>

using namespace hg::engine::accel;
using namespace hg;

VkGraphicsPass::~VkGraphicsPass() noexcept {
    tidy();
}

void VkGraphicsPass::tidy() {

    if (not _vkGraphicsPass) {
        return;
    }

    const auto* const device = Engine::getEngine()->getGraphics()->getCurrentDevice().get();
    device->vkDevice().destroyRenderPass(reinterpret_cast<VkRenderPass>(_STD exchange(_vkGraphicsPass, nullptr)));

    // Will be done implicitly by destructor
    // _attachments.clear();
    // _references.clear();
    // _dependencies.clear();
    // _correlationMasks.clear();
    // _viewMasks.clear();
}

bool VkGraphicsPass::compatible(cref<AccelerationPipeline> pipeline_) const noexcept {

    const auto vkPipe = Cast<VkGraphicsPipeline>(&pipeline_);
    if (not vkPipe) {
        return false;
    }

    // TODO:
    return true;
}

#include "RenderStageNode.hpp"

#include "HORenderPass.hpp"
#include "RenderStagePass.hpp"
#include "../Scene/SceneNodeModel.hpp"
#include "../Device/Device.hpp"

using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

void RenderStageNode::setup(cref<sptr<Device>> device_) {}

void RenderStageNode::destroy() {}

bool RenderStageNode::allocate(const ptr<HORenderPass> renderPass_) {
    return false;
}

bool RenderStageNode::free(const ptr<HORenderPass> renderPass_) {
    return false;
}

const non_owning_rptr<const Vector<type_id>> RenderStageNode::modelTypes() const noexcept {
    return nullptr;
}

void RenderStageNode::before(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {}

void RenderStageNode::invoke(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_,
    const non_owning_rptr<SceneNodeModel> model_
) const {}

void RenderStageNode::after(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {}

#include "RenderStage.hpp"

#include <Engine.GFX/Device/Device.hpp>

#include "../RenderPipeline_Deprecated.hpp"

using namespace hg::engine::gfx::render;
using namespace hg::engine::gfx;
using namespace hg;

void RenderStage::pipelineSetup(const non_owning_rptr<const RenderPipeline_Deprecated> pipeline_) {

    const auto device { pipeline_->device() };

    setup(device);
    _multiplexer.setup(device);
}

void RenderStage::pipelineDestroy(const non_owning_rptr<const RenderPipeline_Deprecated> pipeline_) {
    destroy();
}

void RenderStage::setup(cref<sptr<Device>> device_) {}

void RenderStage::destroy() {}

bool RenderStage::allocate(const ptr<HORenderPass> renderPass_) {
    return _multiplexer.allocate(renderPass_);
}

void RenderStage::free(const ptr<HORenderPass> renderPass_) {
    _multiplexer.free(renderPass_);
}

cref<Vector<RenderStageDependency>> RenderStage::dependencies() const noexcept {
    return _dependencies;
}

ref<Vector<RenderStageDependency>> RenderStage::dependencies() noexcept {
    return _dependencies;
}

bool RenderStage::pushDependency(cref<RenderStageDependency> dependency_) {

    const auto it {
        std::ranges::find_if(
            _dependencies.begin(),
            _dependencies.end(),
            [&dependency_](cref<RenderStageDependency> entry_) {
                return entry_.stage == dependency_.stage;
            }
        )
    };

    if (it != _dependencies.end()) {
        return false;
    }

    _dependencies.push_back(dependency_);
    return true;
}

ref<Multiplexer> RenderStage::multiplexer() noexcept {
    return _multiplexer;
}

cref<Multiplexer> RenderStage::multiplexer() const noexcept {
    return _multiplexer;
}

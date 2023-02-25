#include "RenderScenePipeline.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Core.Schedule/CorePipeline.hpp>
#include <Engine.Reflow.Schedule/ReflowPipeline.hpp>
#include <Engine.Scheduler/Tick/TickPipeline.hpp>

#include "RenderTickStage.hpp"

using namespace ember::engine::gfx::schedule;
using namespace ember::engine::gfx;
using namespace ember::engine::scheduler;
using namespace ember;

RenderScenePipeline::RenderScenePipeline() :
    StagePipeline(identifier_type::from("::Gfx::RenderScenePipeline")) {}

RenderScenePipeline::~RenderScenePipeline() = default;

void RenderScenePipeline::mount(const non_owning_rptr<StageRegister> register_) {

    const auto* const renderTick = register_->registerStage(
        make_uptr<RenderTickStage>(RenderTick, this)
    );
    _orderedStages.push_back(renderTick);
}

void RenderScenePipeline::declareDependencies(
    const non_owning_rptr<const StageRegister> register_,
    ref<CompactSet<StageDependency>> collection_
) {
    const auto* const beginTick = register_->getStage(TickPipeline::TickBegin);
    const auto* const actorUpdate = register_->getStage(core::schedule::CorePipeline::ActorUpdate);
    const auto* const shiftTick = register_->getStage(reflow::schedule::ReflowPipeline::ShiftTick);
    // TODO:
    const auto* const physicsTick = register_->getStage("__PhysicsTick__");
    const auto* const animatorTick = register_->getStage("__AnimatorTick__");
    const auto* const ikTick = register_->getStage("__IKTick__");

    const auto* const renderTick = _orderedStages.front();

    collection_.insert(
        StageDependency {
            { beginTick, actorUpdate, shiftTick, /* TODO: actorTick, physicsTick, animatorTick, ikTick */ },
            this,
            renderTick
        }
    );
}

void RenderScenePipeline::dismount(const non_owning_rptr<StageRegister> register_) {
    register_->removeStage(RenderTick);
    _orderedStages.clear();
}

bool RenderScenePipeline::isSkippable() const noexcept {
    return false;
}

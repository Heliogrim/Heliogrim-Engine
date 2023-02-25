#include "ReflowPipeline.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Core.Schedule/CorePipeline.hpp>
#include <Engine.Input.Schedule/InputPipeline.hpp>
#include <Engine.Scheduler/Tick/TickPipeline.hpp>

#include "ReflowFlowStage.hpp"
#include "ReflowShiftStage.hpp"

using namespace ember::engine::reflow::schedule;
using namespace ember::engine::scheduler;
using namespace ember;

ReflowPipeline::ReflowPipeline() :
    StagePipeline(identifier_type::from("::Reflow::ReflowPipeline")) {}

ReflowPipeline::~ReflowPipeline() = default;

void ReflowPipeline::mount(const non_owning_rptr<scheduler::StageRegister> register_) {

    const auto* const flowTick = register_->registerStage(
        make_uptr<ReflowFlowStage>(FlowTick, this)
    );
    const auto* const shiftTick = register_->registerStage(
        make_uptr<ReflowShiftStage>(ShiftTick, this)
    );

    /**/

    _orderedStages.push_back(flowTick);
    _orderedStages.push_back(shiftTick);
}

void ReflowPipeline::declareDependencies(
    const non_owning_rptr<const scheduler::StageRegister> register_,
    ref<CompactSet<StageDependency>> collection_
) {
    const auto* const beginTick = register_->getStage(TickPipeline::TickBegin);
    const auto* const actorUpdate = register_->getStage(core::schedule::CorePipeline::ActorUpdate);
    const auto* const inputTick = register_->getStage(input::schedule::InputPipeline::InputTick);

    /**/

    const auto* const flowTick = _orderedStages.front();

    /**/

    collection_.insert(
        StageDependency {
            { beginTick, actorUpdate, inputTick },
            this,
            flowTick
        }
    );
}

void ReflowPipeline::dismount(const non_owning_rptr<scheduler::StageRegister> register_) {

    register_->removeStage(FlowTick);
    register_->removeStage(ShiftTick);

    /**/

    _orderedStages.clear();
}

bool ReflowPipeline::isSkippable() const noexcept {
    return false;
}

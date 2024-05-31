#include "ReflowPipeline.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Core.Schedule/CorePipeline.hpp>
#include <Engine.Input.Schedule/InputPipeline.hpp>
#include <Engine.Scheduler/Tick/TickPipeline.hpp>

#include "ReflowFlowStage.hpp"

using namespace hg::engine::reflow::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

ReflowPipeline::ReflowPipeline() :
    StagePipeline(identifier_type::from("::Reflow::ReflowPipeline")) {}

ReflowPipeline::~ReflowPipeline() = default;

void ReflowPipeline::mount(ref<StageRegister> register_) {

    const auto* const tick = register_.registerStage(
        make_uptr<ReflowFlowStage>(Tick, this)
    );

    /**/

    _orderedStages.push_back(tick);
}

void ReflowPipeline::declareDependencies(
    cref<StageRegister> register_,
    ref<CompactSet<StageDependency>> collection_
) {
    const auto* const beginTick = register_.getStage(TickPipeline::TickBegin);
    const auto* const actorUpdate = register_.getStage(core::schedule::CorePipeline::ActorUpdate);
    const auto* const inputTick = register_.getStage(input::schedule::InputPipeline::InputTick);

    /**/

    const auto* const tick = _orderedStages.front();

    /**/

    collection_.insert(
        StageDependency {
            { beginTick, actorUpdate, inputTick },
            this,
            tick
        }
    );
}

void ReflowPipeline::dismount(ref<StageRegister> register_) {

    register_.removeStage(Tick);

    /**/

    _orderedStages.clear();
}

bool ReflowPipeline::isSkippable() const noexcept {
    return false;
}

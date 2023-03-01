#include "CorePipeline.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Input.Schedule/InputPipeline.hpp>
#include <Engine.Scheduler/Tick/TickPipeline.hpp>

#include "ActorUpdateStage.hpp"

using namespace hg::engine::core::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

CorePipeline::CorePipeline() :
    StagePipeline(identifier_type::from("::Core::CorePipeline")) {}

CorePipeline::~CorePipeline() = default;

void CorePipeline::mount(const non_owning_rptr<scheduler::StageRegister> register_) {

    const auto actorUpdate = register_->registerStage(
        make_uptr<ActorUpdateStage>(ActorUpdate, this)
    );

    /**/

    _orderedStages.push_back(actorUpdate);
}

void CorePipeline::declareDependencies(
    const non_owning_rptr<const scheduler::StageRegister> register_,
    ref<CompactSet<StageDependency>> collection_
) {
    const auto* const beginTick = register_->getStage(TickPipeline::TickBegin);
    const auto* const inputTick = register_->getStage(input::schedule::InputPipeline::InputTick);

    /**/

    const auto* const actorUpdate = _orderedStages.front();

    /**/

    collection_.insert(
        StageDependency {
            { beginTick, inputTick },
            this,
            actorUpdate
        }
    );
}

void CorePipeline::dismount(const non_owning_rptr<scheduler::StageRegister> register_) {

    register_->removeStage(ActorUpdate);

    /**/

    _orderedStages.clear();
}

bool CorePipeline::isSkippable() const noexcept {
    return false;
}

#include "TickPipeline.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.Schedule/RenderScenePipeline.hpp>
#include "../Pipeline/Stage/EmptyPipelineStage.hpp"

using namespace ember::engine::scheduler;
using namespace ember;

TickPipeline::TickPipeline() :
    StagePipeline(identifier_type::from("::Scheduler::TickPipeline")) {}

TickPipeline::~TickPipeline() noexcept = default;

void TickPipeline::mount(const non_owning_rptr<StageRegister> register_) {

    const auto beginStage = register_->registerStage(
        make_uptr<EmptyPipelineStage>(PipelineStage::identifier_type::from(TickBegin), this)
    );
    //const auto eventStage = register_->registerStage(TickEvent);
    const auto endStage = register_->registerStage(
        make_uptr<EmptyPipelineStage>(PipelineStage::identifier_type::from(TickEnd), this)
    );

    /**/

    _orderedStages.reserve(2ui64);
    _orderedStages.push_back(beginStage);
    //_orderedStages.push_back(eventStage);
    _orderedStages.push_back(endStage);
}

void TickPipeline::declareDependencies(
    const non_owning_rptr<const StageRegister> register_,
    ref<CompactSet<StageDependency>> collection_
) {

    // TODO: Guarantee that tick end stage is latest within dependent chain
    // Attention: Hotfix

    const auto* const renderScene = register_->getStage(gfx::schedule::RenderScenePipeline::RenderTick);

    /**/

    const auto* const tickEnd = _orderedStages.back();

    /**/

    collection_.insert(
        StageDependency {
            { renderScene },
            this,
            tickEnd
        }
    );
}

void TickPipeline::dismount(const non_owning_rptr<StageRegister> register_) {

    register_->removeStage(TickEnd);
    //register_->removeStage(TickEvent);
    register_->removeStage(TickEnd);

    /**/

    _orderedStages.clear();
}

bool TickPipeline::isSkippable() const noexcept {
    return false;
}

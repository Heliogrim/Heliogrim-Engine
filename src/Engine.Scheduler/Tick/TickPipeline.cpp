#include "TickPipeline.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.Schedule/RenderScenePipeline.hpp>
#include "../Pipeline/Stage/EmptyPipelineStage.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

TickPipeline::TickPipeline() :
    StagePipeline(identifier_type::from("::Scheduler::TickPipeline")) {}

TickPipeline::~TickPipeline() noexcept = default;

void TickPipeline::mount(ref<StageRegister> register_) {

    const auto* const beginStage = register_.registerStage(
        make_uptr<EmptyPipelineStage>(PipelineStage::identifier_type::from(TickBegin), this)
    );
    //const auto eventStage = register_->registerStage(TickEvent);
    const auto* const endStage = register_.registerStage(
        make_uptr<EmptyPipelineStage>(PipelineStage::identifier_type::from(TickEnd), this)
    );

    /**/

    _orderedStages.reserve(2uLL);
    _orderedStages.push_back(beginStage);
    //_orderedStages.push_back(eventStage);
    _orderedStages.push_back(endStage);
}

void TickPipeline::declareDependencies(
    cref<StageRegister> register_,
    ref<CompactSet<StageDependency>> collection_
) {

    // TODO: Guarantee that tick end stage is latest within dependent chain
    // Attention: Hotfix

    const auto* const renderScene = register_.getStage(gfx::schedule::RenderScenePipeline::RenderTick);

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

void TickPipeline::dismount(ref<StageRegister> register_) {

    register_.removeStage(TickEnd);
    //register_->removeStage(TickEvent);
    register_.removeStage(TickEnd);

    /**/

    _orderedStages.clear();
}

bool TickPipeline::isSkippable() const noexcept {
    return false;
}

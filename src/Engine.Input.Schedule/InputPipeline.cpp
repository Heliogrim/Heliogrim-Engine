#include "InputPipeline.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Scheduler/Tick/TickPipeline.hpp>

#include "InputTickStage.hpp"

using namespace ember::engine::input::schedule;
using namespace ember::engine::scheduler;
using namespace ember;

InputPipeline::InputPipeline() :
    StagePipeline(identifier_type::from("::Input::InputPipeline")) {}

InputPipeline::~InputPipeline() = default;

void InputPipeline::mount(const non_owning_rptr<scheduler::StageRegister> register_) {

    const auto* const inputTick = register_->registerStage(
        make_uptr<InputTickStage>(InputTick, this)
    );

    /**/

    _orderedStages.push_back(inputTick);
}

void InputPipeline::declareDependencies(
    const non_owning_rptr<const scheduler::StageRegister> register_,
    ref<CompactSet<StageDependency>> collection_
) {
    const auto* const beginTick = register_->getStage(TickPipeline::TickBegin);

    /**/

    const auto* const inputTick = _orderedStages.front();

    /**/

    collection_.insert(
        StageDependency {
            { beginTick },
            this,
            inputTick
        }
    );
}

void InputPipeline::dismount(const non_owning_rptr<scheduler::StageRegister> register_) {

    register_->removeStage(InputTick);

    /**/

    _orderedStages.clear();
}

bool InputPipeline::isSkippable() const noexcept {
    return false;
}

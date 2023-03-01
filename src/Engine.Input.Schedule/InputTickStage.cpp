#include "InputTickStage.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.Input/Input.hpp>

using namespace hg::engine::input::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

InputTickStage::InputTickStage(
    cref<StageIdentifier> identifier_,
    pipeline_handle_type pipeline_
) :
    PipelineStage(identifier_type::from(identifier_), pipeline_) {}

InputTickStage::~InputTickStage() = default;

void InputTickStage::staticDispatch(const non_owning_rptr<const scheduler::StageDispatcher> dispatcher_) {
    dispatcher_->enqueue(
        task::make_repetitive_task(
            []() {
                Engine::getEngine()->getInput()->tick();
                return true;
            }
        )
    );
}

void InputTickStage::dynamicDispatch(const non_owning_rptr<const scheduler::StageDispatcher> dispatcher_) {
    // TODO:
}

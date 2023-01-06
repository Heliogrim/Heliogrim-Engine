#include "ReflowShiftStage.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.Reflow/Window/BoundWindow.hpp>
#include <Engine.Reflow/Window/WindowManager.hpp>

using namespace ember::engine::reflow::schedule;
using namespace ember::engine::scheduler;
using namespace ember;

ReflowShiftStage::ReflowShiftStage(
    cref<StageIdentifier> identifier_,
    pipeline_handle_type pipeline_
) :
    PipelineStage(identifier_type::from(identifier_), pipeline_) {}

ReflowShiftStage::~ReflowShiftStage() = default;

void ReflowShiftStage::staticDispatch(const non_owning_rptr<const scheduler::StageDispatcher> dispatcher_) {
    dispatcher_->enqueue(task::make_repetitive_task([]() {

        const auto* const engine = Engine::getEngine();
        const auto& manager = ::ember::engine::reflow::WindowManager::get();

        if (!manager) {
            // TODO: return false;
            return true;
        }

        /**/

        const auto& wnds = manager->_windows;
        for (const auto& boundWnd : wnds) {

            SCOPED_STOPWATCH_V(__reflow__singleShift)

            const auto clientSize = boundWnd->window->outerSize();

            reflow::FlowContext ctx {
                math::fExtent2D { clientSize.x, clientSize.y, 0.F, 0.F },
                math::fExtent2D { clientSize.x, clientSize.y, 0.F, 0.F }
            };
            math::vec2 offset {};

            boundWnd->window->shift(ctx, offset);
        }

        return true;
    }));
}

void ReflowShiftStage::dynamicDispatch(const non_owning_rptr<const scheduler::StageDispatcher> dispatcher_) {
    // TODO:
}

#include "ReflowFlowStage.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.Reflow/Window/BoundWindow.hpp>
#include <Engine.Reflow/Window/WindowManager.hpp>

using namespace hg::engine::reflow::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

ReflowFlowStage::ReflowFlowStage(
    cref<StageIdentifier> identifier_,
    pipeline_handle_type pipeline_
) :
    PipelineStage(identifier_type::from(identifier_), pipeline_) {}

ReflowFlowStage::~ReflowFlowStage() = default;

void ReflowFlowStage::staticDispatch(const non_owning_rptr<const scheduler::StageDispatcher> dispatcher_) {
    dispatcher_->enqueue(
        task::make_repetitive_task(
            []() {

                const auto* const engine = Engine::getEngine();
                const auto& manager = ::hg::engine::reflow::WindowManager::get();

                if (!manager) {
                    // TODO: return false;
                    return true;
                }

                /**/

                const auto& wnds = manager->_windows;
                for (const auto& boundWnd : wnds) {

                    SCOPED_STOPWATCH_V(__reflow__singleFlow)

                    const auto clientSize = boundWnd->window->outerSize();

                    reflow::FlowContext ctx {
                        math::fExtent2D { clientSize.x, clientSize.y, 0.F, 0.F },
                        math::fExtent2D { clientSize.x, clientSize.y, 0.F, 0.F }
                    };

                    reflow::StyleKeyStack stack {};
                    boundWnd->window->flow(ctx, clientSize, clientSize, stack);
                }

                return true;
            }
        )
    );
}

void ReflowFlowStage::dynamicDispatch(const non_owning_rptr<const scheduler::StageDispatcher> dispatcher_) {
    // TODO:
}

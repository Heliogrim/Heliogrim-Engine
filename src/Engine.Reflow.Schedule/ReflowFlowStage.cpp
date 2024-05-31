#include "ReflowFlowStage.hpp"

#include <chrono>

#include <Engine.Core/Engine.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Reflow/Window/BoundWindow.hpp>
#include <Engine.Reflow/Window/WindowManager.hpp>

#include <Engine.Reflow/ReflowEngine.hpp>
#include <Engine.Reflow/ReflowState.hpp>

#include "Engine.GFX/Surface/Surface.hpp"

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

				const auto engine = Engine::getEngine();
				const auto& manager = ::hg::engine::reflow::WindowManager::get();

				ReflowEngine::updateTickVersion();

				/**/

				if (!manager) {
					// TODO: return false;
					return true;
				}

				/**/

				const auto& wnds = manager->_windows;
				for (const auto& boundWnd : wnds) {

					SCOPED_STOPWATCH_V(__reflow__singleFlow)

					/*
					reflow::StyleKeyStack stack {};
					boundWnd->window->flow(ctx, clientSize, clientSize, stack);
					 */

					const math::vec2 clientSize = boundWnd->window->getClientSize();
					auto start = std::chrono::high_resolution_clock::now();

					ReflowState state {};
					auto layoutContext = reflow::LayoutContext {
						math::vec2 { 0.F },
						math::vec2 { clientSize.x, clientSize.y },
						1.F
					};

					state.setRenderTick(ReflowEngine::getGlobalRenderTick());
					ReflowEngine::tick(state, boundWnd->window, std::move(layoutContext));

					auto end = std::chrono::high_resolution_clock::now();
					IM_DEBUG_LOGF(
						"Next Flex-Flow took: {}",
						std::chrono::duration_cast<std::chrono::microseconds>(end - start)
					);
				}

				return true;
			}
		)
	);
}

void ReflowFlowStage::dynamicDispatch(const non_owning_rptr<const scheduler::StageDispatcher> dispatcher_) {
	// TODO:
}

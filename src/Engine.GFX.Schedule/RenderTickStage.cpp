#include "RenderTickStage.hpp"

#include <chrono>
#include <Engine.Common/Concurrent/Collection/RingBuffer.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX.Scene/RenderSceneManager.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX/RenderEnqueueResult.hpp>
#include <Engine.GFX/RenderTarget.hpp>
#include <Engine.Logging/Logger.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace hg::engine::gfx::schedule;
using namespace hg::engine::scheduler;
using namespace hg;

RenderTickStage::RenderTickStage(
	cref<StageIdentifier> identifier_,
	pipeline_handle_type pipeline_
) :
	PipelineStage(identifier_type::from(identifier_), pipeline_) {}

RenderTickStage::~RenderTickStage() = default;

void RenderTickStage::staticDispatch(cref<StaticStageDispatcher> dispatcher_) {
	dispatcher_.enqueue(
		task::make_repetitive_task(
			[this]() {
				tickTargets();
				return true;
			}
		)
	);
}

void RenderTickStage::dynamicDispatch(cref<DynamicStageDispatcher> dispatcher_) {
	// TODO:
}

void RenderTickStage::tickTargets() {
	CompactSet<smr<RenderTarget>> targets {};
	Engine::getEngine()->getGraphics()->getSceneManager()->selectInvokeTargets(targets);

	for (const auto& target : targets) {
		invokeRenderTarget(target);
	}

	/**
	 * Debug Metrics
	 */
	static concurrent::RingBuffer<std::chrono::nanoseconds> timeBuffer { 17 };

	static auto lastFrame = std::chrono::high_resolution_clock::now();
	const auto now = std::chrono::high_resolution_clock::now();

	auto duration { std::chrono::duration_cast<std::chrono::nanoseconds>(now - lastFrame) };
	timeBuffer.try_push(std::move(duration));

	if (timeBuffer.full()) {
		std::chrono::nanoseconds tmp {};
		timeBuffer.try_pop(tmp);
	}

	lastFrame = now;

	/**/

	std::chrono::nanoseconds accValue {};
	u32 accCount { 0uL };
	for (const auto& entry : timeBuffer.unsafe_container()) {
		if (entry.count() > 0) {
			accValue += entry;
			++accCount;
		}
	}

	const double avg { static_cast<double>(accValue.count()) / static_cast<double>(accCount) };
	const double diff { static_cast<double>(duration.count()) / 1000'000.0 };

	// TODO: Capture report stats : reportStats(static_cast<float>(1000'000'000.0 / avg), static_cast<float>(diff));
}

void RenderTickStage::invokeRenderTarget(cref<smr<RenderTarget>> target_) const {
	SCOPED_STOPWATCH

	/**
	 * Tick RenderTarget
	 */
	if (not target_->ready()) {
		IM_DEBUG_LOG(
			"Tried to tick an unready RenderTarget. Please ensure the completness of RenderTargets before ticking/dispatching..."
		);
		return;
	}

	const auto renderPass = target_->next();

	if (not renderPass) {
		IM_DEBUG_LOG("Skipping graphics tick due to missing RenderPass (No next Swapchain Image) at RenderTarget");
		return;
	}

	target_->update();

	const auto result = target_->finish({});
	assert(result != RenderEnqueueResult::eFailed);
}

#pragma once
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Scheduler/Pipeline/Stage/PipelineStage.hpp>

namespace hg::engine::gfx {
	class RenderTarget;
}

namespace hg::engine::gfx::schedule {
	class RenderTickStage final :
		public scheduler::PipelineStage {
	public:
		using this_type = RenderTickStage;

	public:
		RenderTickStage(
			cref<scheduler::StageIdentifier> identifier_,
			pipeline_handle_type pipeline_
		);

		~RenderTickStage() override;

	public:
		void staticDispatch(cref<scheduler::StaticStageDispatcher> dispatcher_) override;

		void dynamicDispatch(cref<scheduler::DynamicStageDispatcher> dispatcher_) override;

	private:
		void tickTargets();

		void invokeRenderTarget(cref<smr<RenderTarget>> target_) const;
	};
}

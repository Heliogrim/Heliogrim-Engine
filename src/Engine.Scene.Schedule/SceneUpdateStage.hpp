#pragma once

#include <Engine.Scheduler/Pipeline/Stage/PipelineStage.hpp>

namespace hg::engine::scene::schedule {
	class SceneUpdateStage final :
		public scheduler::PipelineStage {
	public:
		using this_type = SceneUpdateStage;

	public:
		SceneUpdateStage(
			cref<scheduler::StageIdentifier> identifier_,
			pipeline_handle_type pipeline_
		);

		~SceneUpdateStage() override;

	public:
		void staticDispatch(cref<scheduler::StaticStageDispatcher> dispatcher_) override;

		void dynamicDispatch(cref<scheduler::DynamicStageDispatcher> dispatcher_) override;
	};
}

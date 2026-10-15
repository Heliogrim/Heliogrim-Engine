#pragma once

#include <Engine.Scheduler/Pipeline/Stage/PipelineStage.hpp>

namespace hg::engine::physics::schedule {
	class PhysicsTickStage final :
		public scheduler::PipelineStage {
	public:
		using this_type = PhysicsTickStage;

	public:
		PhysicsTickStage(ref<const scheduler::StageIdentifier> identifier_, pipeline_handle_type pipeline_);

		~PhysicsTickStage() override;

	public:
		void staticDispatch(cref<scheduler::StaticStageDispatcher> dispatcher_) override;

		void dynamicDispatch(cref<scheduler::DynamicStageDispatcher> dispatcher_) override;
	};
}

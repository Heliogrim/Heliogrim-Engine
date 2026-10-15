#pragma once

#include <Engine.Scheduler/Pipeline/Stage/PipelineStage.hpp>

namespace hg::engine::acs::schedule {
	class ActorPhysicsStageCollision final :
		public scheduler::PipelineStage {
	public:
		using this_type = ActorPhysicsStageCollision;

	public:
		ActorPhysicsStageCollision(ref<const scheduler::StageIdentifier> identifier_, pipeline_handle_type pipeline_);

		~ActorPhysicsStageCollision() override;

	public:
		void staticDispatch(ref<const scheduler::StaticStageDispatcher> dispatcher_) override;

		void dynamicDispatch(ref<const scheduler::DynamicStageDispatcher> dispatcher_) override;
	};
}

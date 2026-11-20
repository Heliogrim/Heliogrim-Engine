#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Scheduler;
#endif

namespace hg::engine::acs::schedule {
	class ActorHousekeepingStage :
		public scheduler::PipelineStage {
	public:
		using this_type = ActorHousekeepingStage;

	public:
		ActorHousekeepingStage(
			ref<const scheduler::StageIdentifier> identifier_,
			pipeline_handle_type pipeline_
		);

		~ActorHousekeepingStage() override;

	public:
		void staticDispatch(ref<const scheduler::StaticStageDispatcher> dispatcher_) override;

		void dynamicDispatch(ref<const scheduler::DynamicStageDispatcher> dispatcher_) override;

		[[nodiscard]] scheduler::graph::StageDecl decl() const noexcept override;
	};
}

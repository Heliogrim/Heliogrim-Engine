#pragma once
#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Scheduler;
#endif

namespace hg::engine::acs::schedule {
	class ActorTickStage final :
		public scheduler::PipelineStage {
	public:
		using this_type = ActorTickStage;

	public:
		ActorTickStage(
			cref<scheduler::StageIdentifier> identifier_,
			pipeline_handle_type pipeline_
		);

		~ActorTickStage() override;

	public:
		void staticDispatch(cref<scheduler::StaticStageDispatcher> dispatcher_) override;

		void dynamicDispatch(cref<scheduler::DynamicStageDispatcher> dispatcher_) override;
	};
}

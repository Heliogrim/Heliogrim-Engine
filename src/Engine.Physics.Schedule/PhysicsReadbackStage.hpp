#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Scheduler;
#endif

namespace hg::engine::physics::schedule {
	class PhysicsReadbackStage final :
		public scheduler::PipelineStage {
	public:
		using this_type = PhysicsReadbackStage;

	public:
		PhysicsReadbackStage(ref<const scheduler::StageIdentifier> identifier_, pipeline_handle_type pipeline_);

		~PhysicsReadbackStage() override;

	public:
		void staticDispatch(ref<const scheduler::StaticStageDispatcher> dispatcher_) override;

		void dynamicDispatch(ref<const scheduler::DynamicStageDispatcher> dispatcher_) override;

		[[nodiscard]] scheduler::graph::StageDecl decl() const noexcept override;
	};
}

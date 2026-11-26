#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Scheduler;
#endif

namespace hg::engine::core::schedule {
	class TeardownStage final :
		public scheduler::PipelineStage {
	public:
		using this_type = TeardownStage;

	public:
		TeardownStage(mref<identifier_type> identifier_, pipeline_handle_type pipeline_);

		~TeardownStage() noexcept override;

	public:
		void staticDispatch(cref<scheduler::StaticStageDispatcher> dispatcher_) override;

		void dynamicDispatch(cref<scheduler::DynamicStageDispatcher> dispatcher_) override;

		[[nodiscard]] scheduler::graph::StageDecl decl() const noexcept override;
	};
}

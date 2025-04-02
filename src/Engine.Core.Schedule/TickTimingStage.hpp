#pragma once

#include <Engine.Scheduler/Pipeline/Stage/PipelineStage.hpp>

namespace hg::engine::core::schedule {
	class TickTimingStage final :
		public scheduler::PipelineStage {
	public:
		using this_type = TickTimingStage;

	public:
		TickTimingStage(mref<identifier_type> identifier_, pipeline_handle_type pipeline_);

		~TickTimingStage() noexcept override;

	public:
		void staticDispatch(cref<scheduler::StaticStageDispatcher> dispatcher_) override;

		void dynamicDispatch(cref<scheduler::DynamicStageDispatcher> dispatcher_) override;
	};
}

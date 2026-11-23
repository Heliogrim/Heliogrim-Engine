#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Scene;
import Engine.Scheduler;
#endif

namespace hg::engine::scene::schedule {
	struct SceneBroadcastStageOptions {
		SceneBroadcastFlags flags;
	};

	class SceneBroadcastStage final :
		public scheduler::PipelineStage {
	public:
		using this_type = SceneBroadcastStage;

	public:
		SceneBroadcastStage(
			cref<scheduler::StageIdentifier> identifier_,
			pipeline_handle_type pipeline_,
			mref<SceneBroadcastStageOptions> options_
		);

		~SceneBroadcastStage() override;

	private:
		SceneBroadcastStageOptions _options;

	public:
		void staticDispatch(cref<scheduler::StaticStageDispatcher> dispatcher_) override;

		void dynamicDispatch(cref<scheduler::DynamicStageDispatcher> dispatcher_) override;

		[[nodiscard]] scheduler::graph::StageDecl decl() const noexcept override;
	};
}

#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Scheduler;
#endif

namespace hg::engine::input::schedule {
	class InputPipeline final :
		public engine::scheduler::StagePipeline {
	public:
		using this_type = InputPipeline;

		using StageIdentifier = ::hg::engine::scheduler::StageIdentifier;

		/**/

		static const inline StageIdentifier InputTick = "::Input::InputTick";

	public:
		InputPipeline();

		~InputPipeline() override;

	private:
		Array<Opt<ref<const ::hg::engine::scheduler::Stage>>, 1> _stages;

	public:
		void mount(ref<scheduler::StageRegister> register_) override;

		void dismount(ref<scheduler::StageRegister> register_) override;
	};
}

#pragma once

#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Scheduler/Pipeline/StagePipeline.hpp>

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

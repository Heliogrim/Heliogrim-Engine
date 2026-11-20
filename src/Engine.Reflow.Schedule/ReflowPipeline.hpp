#pragma once

#include <Engine.Scheduler/Pipeline/StagePipeline.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow::schedule {
	class ReflowPipeline final :
		public engine::scheduler::StagePipeline {
	public:
		using this_type = ReflowPipeline;

		using StageIdentifier = ::hg::engine::scheduler::StageIdentifier;

		/**/

		static const inline StageIdentifier Tick = "::Reflow::Tick";

	public:
		ReflowPipeline();

		~ReflowPipeline() override;

	private:
		Array<Opt<ref<const ::hg::engine::scheduler::Stage>>, 1> _stages;

	public:
		void mount(ref<scheduler::StageRegister> register_) override;

		void dismount(ref<scheduler::StageRegister> register_) override;
	};
}

#pragma once

#include <Engine.Scheduler/Pipeline/StagePipeline.hpp>

namespace hg::engine::core::schedule {
	class CorePipeline final :
		public scheduler::StagePipeline {
	public:
		using this_type = CorePipeline;

		using StageIdentifier = ::hg::engine::scheduler::StageIdentifier;

		/**/

		static const inline StageIdentifier TickBegin = "::Core::TickBegin";
		static const inline StageIdentifier TickEnd = "::Core::TickEnd";

	public:
		CorePipeline();

		~CorePipeline() override;

	public:
		void mount(ref<scheduler::StageRegister> register_) override;

		void dismount(ref<scheduler::StageRegister> register_) override;
	};
}

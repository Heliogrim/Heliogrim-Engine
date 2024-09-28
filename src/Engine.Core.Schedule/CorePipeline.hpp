#pragma once
#include <Engine.Scheduler/Pipeline/StagePipeline.hpp>

namespace hg::engine::core::schedule {
	class CorePipeline final :
		public scheduler::StagePipeline {
	public:
		using this_type = CorePipeline;

		using StageIdentifier = ::hg::engine::scheduler::StageIdentifier;
		using StageDependency = ::hg::engine::scheduler::StageDependency;

		/**/

		static const inline StageIdentifier TickBegin = "::Core::TickBegin";
		static const inline StageIdentifier TickEnd = "::Core::TickEnd";

	public:
		CorePipeline();

		~CorePipeline() override;

	public:
		void mount(ref<scheduler::StageRegister> register_) override;

		void declareDependencies(
			cref<scheduler::StageRegister> register_,
			ref<CompactSet<StageDependency>> collection_
		) override;

		void dismount(ref<scheduler::StageRegister> register_) override;

	public:
		[[nodiscard]] bool isSkippable() const noexcept override;
	};
}

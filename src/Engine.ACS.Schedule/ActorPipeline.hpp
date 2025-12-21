#pragma once
#include <Engine.Scheduler/Pipeline/StagePipeline.hpp>

namespace hg::engine::acs::schedule {
	class ActorPipeline final :
		public scheduler::StagePipeline {
	public:
		using this_type = ActorPipeline;

		using StageIdentifier = ::hg::engine::scheduler::StageIdentifier;
		using StageDependency = ::hg::engine::scheduler::StageDependency;

		/**/

		static const inline StageIdentifier ActorTick = "::Core::ActorTick";
		static const inline StageIdentifier ActorHousekeeping = "::Core::ActorHousekeeping";

	public:
		ActorPipeline();

		~ActorPipeline() override;

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
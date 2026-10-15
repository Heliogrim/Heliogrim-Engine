#pragma once

#include <Engine.Scheduler/Pipeline/StagePipeline.hpp>
#include <Engine.Scheduler/Pipeline/Stage/StageIdentifier.hpp>

namespace hg::engine::physics::schedule {
	class PhysicsPipeline final :
		public scheduler::StagePipeline {
	public:
		using this_type = PhysicsPipeline;

		using StageIdentifier = ::hg::engine::scheduler::StageIdentifier;
		using StageDependency = ::hg::engine::scheduler::StageDependency;

		/**/

		static const inline StageIdentifier PhysicsTick = "::Physics::Tick";
		static const inline StageIdentifier PhysicsReadback = "::Physics::Readback";

	public:
		PhysicsPipeline();

		~PhysicsPipeline() override;

	public:
		void mount(ref<scheduler::StageRegister> register_) override;

		void declareDependencies(cref<scheduler::StageRegister> register_, ref<CompactSet<StageDependency>> collection_) override;

		void dismount(ref<scheduler::StageRegister> register_) override;

	public:
		[[nodiscard]] bool isSkippable() const noexcept override;
	};
}

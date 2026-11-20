#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Scheduler;
#endif

namespace hg::engine::physics::schedule {
	class PhysicsPipeline final :
		public scheduler::StagePipeline {
	public:
		using this_type = PhysicsPipeline;

		using StageIdentifier = ::hg::engine::scheduler::StageIdentifier;

		/**/

		static const inline StageIdentifier PhysicsTick = "::Physics::Tick";
		static const inline StageIdentifier PhysicsReadback = "::Physics::Readback";

	public:
		PhysicsPipeline();

		~PhysicsPipeline() override;

	private:
		Array<Opt<ref<const ::hg::engine::scheduler::Stage>>, 2> _stages;

	public:
		void mount(ref<scheduler::StageRegister> register_) override;

		void dismount(ref<scheduler::StageRegister> register_) override;
	};
}

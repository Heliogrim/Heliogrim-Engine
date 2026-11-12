#pragma once

#include <Engine.Scheduler/Pipeline/StagePipeline.hpp>

namespace hg::engine::graphics::schedule {
	class RenderScenePipeline final :
		public scheduler::StagePipeline {
	public:
		using this_type = RenderScenePipeline;

		using StageIdentifier = scheduler::StageIdentifier;

		// Tick Begin -> Input Tick -> (Reflow Response) -> Actor Update -> Reflow Tick/Update/Layout -> UI Render | 3D Render | Render Tick -> Tick End
		// ::TickBegin
		// -> ::InputTick
		// -> -> ::ActorUpdate (Core Update)
		// -> -> -> ::ReflowTick
		// -> -> -> -> ::RenderTick
		// -> -> -> -> -> ::TickEnd

		static const inline StageIdentifier RenderTick = "::Graphics::RenderTick";

	public:
		RenderScenePipeline();

		~RenderScenePipeline() override;

	public:
		void mount(ref<scheduler::StageRegister> register_) override;

		void dismount(ref<scheduler::StageRegister> register_) override;
	};
}

#pragma once
#include <Engine.Scheduler/Pipeline/StagePipeline.hpp>

namespace hg::engine::scene::schedule {
	class ScenePipeline final :
		public scheduler::StagePipeline {
	public:
		using this_type = ScenePipeline;

		using StageIdentifier = ::hg::engine::scheduler::StageIdentifier;

		/**/

		static const inline StageIdentifier SceneBroadcastPrePhysics = "::Core::SceneBroadcast::PrePhysics";
		static const inline StageIdentifier SceneBroadcastPostPhysics = "::Core::SceneBroadcast::PostPhysics";
		static const inline StageIdentifier SceneUpdatePrePhysics = "::Core::SceneUpdate::PrePhysics";
		static const inline StageIdentifier SceneUpdatePostPhysics = "::Core::SceneUpdate::PostPhysics";
		static const inline StageIdentifier ScenePostProcess = "::Core::ScenePostProcess";

	public:
		ScenePipeline();

		~ScenePipeline() override;

	public:
		void mount(ref<scheduler::StageRegister> register_) override;

		void dismount(ref<scheduler::StageRegister> register_) override;
	};
}

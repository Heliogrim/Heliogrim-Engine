#pragma once

#include <Editor.Action/Action/DelegateAction.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Variant.hpp>
#include <Engine.Common/Math/Location.hpp>
#include <Engine.Common/Math/Rotator.hpp>
#include <Heliogrim/Actor/ActorGuid.hpp>

namespace hg::editor::scene::actions {
	struct SpawnDirectionalLightActorOptions {
		math::fvec3 luminance;
		math::Rotator direction;
	};

	struct SpawnPointLightActorOptions {};

	struct SpawnSpotLightActorOptions {};

	using SpawnLightActorOptions = Variant<SpawnDirectionalLightActorOptions, SpawnPointLightActorOptions, SpawnSpotLightActorOptions>;

	class SpawnLightActor :
		public DelegateAction {
	public:
		~SpawnLightActor() override;

		SpawnLightActor(
			mref<math::Location> location_,
			mref<SpawnLightActorOptions> options_
		);

	private:
		Opt<ActorGuid> _actorGuid;
	};
}
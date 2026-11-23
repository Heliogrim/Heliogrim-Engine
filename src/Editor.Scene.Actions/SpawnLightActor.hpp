#pragma once

#include <Editor.Action/Action/DelegateAction.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
import Engine.Common;
#endif

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

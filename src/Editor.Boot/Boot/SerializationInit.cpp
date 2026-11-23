/**/

#include <Engine.Common/__macro.hpp>

import Editor.Boot;
import Engine.ACS;
import Engine.Common;
import Engine.Config;
import Engine.Graphics.Components;
import Engine.Serialization;
import Heliogrim.Actors;

using namespace hg;

void editor::boot::initSerialization([[maybe_unused]] cref<engine::Config> cfg_, ref<engine::SerializationModule> module_) {

	/* Actor Schemes */

	module_.addActorScheme<Actor>();
	module_.addActorScheme<CameraActor>();
	module_.addActorScheme(StaticGeometryActor::typeId, ::hg::engine::acs::MakeArchetypeFnSet(&StaticGeometryActor::actorClass()));
	module_.addActorScheme(BoxShapeGeometryActor::typeId, ::hg::engine::acs::MakeArchetypeFnSet(&BoxShapeGeometryActor::actorClass()));
	module_.addActorScheme(SphereShapeGeometryActor::typeId, ::hg::engine::acs::MakeArchetypeFnSet(&SphereShapeGeometryActor::actorClass()));
	module_.addActorScheme<DirectionalLightActor>();
	module_.addActorScheme<PointLightActor>();

	/* Component Schemes */

	module_.addComponentScheme<HierarchyComponent>();
	module_.addComponentScheme<CameraComponent>();
	module_.addComponentScheme<StaticGeometryComponent>();
	module_.addComponentScheme<DirectionalLightComponent>();
	module_.addComponentScheme<PointLightComponent>();
	module_.addComponentScheme<SpotLightComponent>();
	module_.addComponentScheme<SkyboxComponent>();
}

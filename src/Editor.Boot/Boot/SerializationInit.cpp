#include "Engine.ACS/Actor/Actor.hpp"
#include "Engine.ACS/Serialization/ArchetypeFnSet.hpp"
#include "Heliogrim.Actors/Camera/CameraActor.hpp"
#include "Heliogrim.Actors/Geometry/BoxShapeGeometryActor.hpp"
#include "Heliogrim.Actors/Geometry/SphereShapeGeometryActor.hpp"
#include "Heliogrim.Actors/Geometry/StaticGeometryActor.hpp"
#include "Heliogrim.Actors/Light/DirectionalLightActor.hpp"
#include "Heliogrim.Actors/Light/PointLightActor.hpp"
#include "Engine.Graphics.Components/Light/PointLightComponent.hpp"
#include "Engine.Graphics.Components/Light/SpotLightComponent.hpp"
#include "Engine.Graphics.Components/Sky/SkyboxComponent.hpp"

/**/

#include <Engine.ACS/Serialization/ActorScheme.hpp>
#include <Engine.ACS/Serialization/ComponentScheme.hpp>
#include <Engine.ACS/Serialization/SerializationModule.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Serialization/Scheme/DefaultScheme.hpp>
#include <Engine.Serialization/Scheme/Scheme.hpp>

import Editor.Boot;
import Engine.Common;
import Engine.Config;

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

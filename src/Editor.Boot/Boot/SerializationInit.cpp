#include "Heliogrim/Actor/Actor.hpp"
#include "Heliogrim/Actor/Archetype/ArchetypeFnSet.hpp"
#include "Heliogrim/Actor/Camera/CameraActor.hpp"
#include "Heliogrim/Actor/Geometry/BoxShapeGeometryActor.hpp"
#include "Heliogrim/Actor/Geometry/SphereShapeGeometryActor.hpp"
#include "Heliogrim/Actor/Geometry/StaticGeometryActor.hpp"
#include "Heliogrim/Actor/Light/DirectionalLightActor.hpp"
#include "Heliogrim/Actor/Light/PointLightActor.hpp"
#include "Heliogrim/Component/Scene/Light/PointLightComponent.hpp"
#include "Heliogrim/Component/Scene/Light/SpotLightComponent.hpp"
#include "Heliogrim/Component/Scene/Sky/SkyboxComponent.hpp"

/**/

#include "SerializationInit.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Serialization/Scheme/ActorScheme.hpp>
#include <Engine.Serialization/Scheme/ComponentScheme.hpp>
#include <Engine.Serialization/Scheme/DefaultScheme.hpp>
#include <Engine.Serialization/Scheme/Scheme.hpp>
#include <Engine.Serialization/SerializationModule.hpp>

using namespace hg;

void editor::boot::initSerialization([[maybe_unused]] cref<engine::Config> cfg_, ref<engine::SerializationModule> module_) {

	/* Actor Schemes */

	module_.addActorScheme<Actor>();
	module_.addActorScheme<CameraActor>();
	module_.addActorScheme(StaticGeometryActor::typeId, MakeArchetypeFnSet(&StaticGeometryActor::actorClass()));
	module_.addActorScheme(BoxShapeGeometryActor::typeId, MakeArchetypeFnSet(&BoxShapeGeometryActor::actorClass()));
	module_.addActorScheme(SphereShapeGeometryActor::typeId, MakeArchetypeFnSet(&SphereShapeGeometryActor::actorClass()));
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

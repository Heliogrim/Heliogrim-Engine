#include "EditorSceneViewport.hpp"

#include <Editor.Assets.Default/GfxMaterials/Default.hpp>
#include <Engine.ACS/ActorModule.hpp>
#include <Heliogrim/Actor/Camera/CameraActor.hpp>

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

/**/

static void experimental_add_actor(math::Transform baseTransform_);

/**/

EventResponse EditorSceneViewport::invokeOnKeyDown(ref<const KeyboardEvent> event_) {

	if (event_._down == true && event_._key == 'n') {

		auto cameraTransform = _cameraActor->getRootComponent()->getLocalTransform();
		experimental_add_actor(cameraTransform);

		markAsPending();
		return EventResponse::eConsumed;
	}

	return Viewport::invokeOnKeyDown(event_);
}

/**/

#include <Editor.Assets.Default/GfxMaterials/DefaultBrdfMaterial.hpp>
#include <Editor.Assets.Default/Meshes/Default.hpp>
#include <Editor.Core/EditorEngine.hpp>
#include <Heliogrim/ActorInitializer.hpp>
#include <Heliogrim/Heliogrim.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Actor/Geometry/StaticGeometryActor.hpp>
#include <Heliogrim/Core/Session.hpp>
#include <Heliogrim/Universe/Universe.hpp>

void experimental_add_actor(math::Transform baseTransform_) {

	/* Generate actor */

	auto session = GetSession();
	auto universe = GetUniverse(session);

	auto actor = CreateActor();
	auto component = ActorInitializer { *engine::Engine::getEngine()->getActors()->getRegistry() }
		.createComponent<StaticGeometryComponent>(actor.get());

	/* Configure components */

	auto query = Heliogrim::assets()[editor::assets::meshes::default_sphere_guid];
	::hg::assertrt(query.flags & AssetDatabaseResultType::eSuccess);
	component->setStaticGeometryByAsset(static_cast<ref<StaticGeometryAsset>>(query.value));

	query = Heliogrim::assets()[editor::assets::materials::default_brdf_guid];
	::hg::assertrt(query.flags & AssetDatabaseResultType::eSuccess);
	const_cast<CompactArray<GfxMaterialAsset>&>(component->overrideMaterials()).emplace_back(
		static_cast<ref<GfxMaterialAsset>>(query.value)
	);

	/**/

	const_cast<Transform&>(actor->getRootComponent()->getUniverseTransform()).location() = baseTransform_.location();

	/**/

	universe.addActor(std::move(actor));
}

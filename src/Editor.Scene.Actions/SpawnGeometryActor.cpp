#include "SpawnGeometryActor.hpp"

using namespace hg::editor::scene::actions;
using namespace hg;

/**/

[[nodiscard]] static Opt<ActorGuid> spawnActorImpl(
	TypedAssetGuid<engine::assets::StaticGeometry> assetGuid_,
	ref<const math::Location> location_
);

static void despawnActorImpl(mref<ActorGuid> actorGuid_);

/**/

SpawnGeometryActor::SpawnGeometryActor(TypedAssetGuid<engine::assets::StaticGeometry> assetGuid_, mref<math::Location> location_) :
	DelegateAction(
		[this, assetGuid_, location_] {
			_actorGuid = spawnActorImpl(assetGuid_, location_);
		},
		[this] {
			if (_actorGuid != None) {
				despawnActorImpl(*std::exchange(_actorGuid, None));
			}
		}
	) {}

SpawnGeometryActor::~SpawnGeometryActor() = default;

/**/

#include <Editor.Assets.Default/GfxMaterials/Default.hpp>
#include <Editor.Assets.Default/Meshes/Default.hpp>
#include <Editor.Core/EditorEngine.hpp>
#include <Editor.UI.Main/EditorUI.hpp>
#include <Editor.UI.Main/Module/EditorUI.hpp>
#include <Engine.ACS/ActorModule.hpp>
#include <Engine.Common/Discard.hpp>
#include <Heliogrim/ActorInitializer.hpp>
#include <Heliogrim/Heliogrim.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Actor/Geometry/StaticGeometryActor.hpp>
#include <Heliogrim/Asset/GeometryAssetHandles.hpp>
#include <Heliogrim/Asset/MaterialAssetHandles.hpp>
#include <Heliogrim/Core/Session.hpp>
#include <Heliogrim/Universe/Universe.hpp>

Opt<ActorGuid> spawnActorImpl(TypedAssetGuid<engine::assets::StaticGeometry> assetGuid_, ref<const math::Location> location_) {

	/* Generate actor */

	auto session = GetSession();
	auto universe = GetUniverse(session);

	auto volatileActor = CreateActor();
	auto component = volatileActor->addComponent<StaticGeometryComponent>();

	/* Configure components */

	const auto meshQuery = Heliogrim::assets().find<StaticGeometryAssetHandle>(assetGuid_);
	::hg::assertrt(meshQuery.flags & AssetDatabaseResultType::eSuccess);
	component->setStaticGeometry(meshQuery.value);

	const auto materialQuery = Heliogrim::assets().find<GfxMaterialAssetHandle>(editor::assets::materials::default_brdf_guid);
	::hg::assertrt(materialQuery.flags & AssetDatabaseResultType::eSuccess);
	component->setInstanceMaterial(0u, materialQuery.value);

	/**/

	volatileActor->getRootComponent()->getLocalTransform().location() = location_;
	const auto* const actor = universe.addActor(std::move(volatileActor));

	/**/

	auto* const editorUi = static_cast<editor::EditorUI*>(
		editor::EditorEngine::getEngine()->getModules().getSubModule(editor::EditorUIDepKey).get()
	);
	editorUi->getEditorModel().scene.instance.next(universe.unwrap().get());

	/**/

	return Some(actor->guid());
}

void despawnActorImpl(mref<ActorGuid> actorGuid_) {

	/* Generate actor */

	auto session = GetSession();
	auto universe = GetUniverse(session);

	auto* actor = engine::Engine::getEngine()->getActors()->getRegistry()->getOrCreateActorPool<Actor>()->get(actorGuid_);
	universe.dropActor(::hg::move(actor));

	/**/

	auto* const editorUi = static_cast<editor::EditorUI*>(
		editor::EditorEngine::getEngine()->getModules().getSubModule(editor::EditorUIDepKey).get()
	);
	editorUi->getEditorModel().scene.instance.next(universe.unwrap().get());
}

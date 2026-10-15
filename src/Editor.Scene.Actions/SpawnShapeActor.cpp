#include "SpawnShapeActor.hpp"

using namespace hg::editor::scene::actions;
using namespace hg;

/**/

[[nodiscard]] static Opt<ActorGuid> spawnActorImpl(
	SpawnShapeActor::ShapeType shapeType_,
	ref<const math::Location> location_
);

static void despawnActorImpl(mref<ActorGuid> actorGuid_);

/**/

SpawnShapeActor::SpawnShapeActor(ShapeType shapeType_, mref<math::Location> location_) :
	DelegateAction(
		[this, shapeType_, location_] {
			_actorGuid = spawnActorImpl(shapeType_, location_);
		},
		[this] {
			if (_actorGuid != None) {
				despawnActorImpl(*std::exchange(_actorGuid, None));
			}
		}
	) {}

SpawnShapeActor::~SpawnShapeActor() = default;

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
#include <Heliogrim/Actor/Geometry/BoxShapeGeometryActor.hpp>
#include <Heliogrim/Actor/Geometry/SphereShapeGeometryActor.hpp>
#include <Heliogrim/Asset/MaterialAssetHandles.hpp>
#include <Heliogrim/Core/Session.hpp>
#include <Heliogrim/Universe/Universe.hpp>

Opt<ActorGuid> spawnActorImpl(SpawnShapeActor::ShapeType shapeType_, ref<const math::Location> location_) {

	/* Generate actor */

	auto session = GetSession();
	auto universe = GetUniverse(session);

	const ptr<const ActorClass> actorClass = shapeType_ == SpawnShapeActor::ShapeType::eBox ?
		static_cast<ptr<const ActorClass>>(BoxShapeGeometryActor::getStaticMetaClass<>()) :
		static_cast<ptr<const ActorClass>>(SphereShapeGeometryActor::getStaticMetaClass<>());
	auto volatileActor = CreateActor(actorClass);

	/* Configure components */

	const auto materialQuery = Heliogrim::assets().find<GfxMaterialAssetHandle>(editor::assets::materials::default_brdf_guid);
	::hg::assertrt(materialQuery.flags & AssetDatabaseResultType::eSuccess);

	if (shapeType_ == SpawnShapeActor::ShapeType::eBox) {
		const auto& boxActor = *Cast<BoxShapeGeometryActor>(volatileActor.get());
		auto& renderComponent = *boxActor.getBoxShapeGeometryComponent();
		renderComponent.setInstanceMaterial(materialQuery.value);

	} else if (shapeType_ == SpawnShapeActor::ShapeType::eSphere) {
		const auto& sphereActor = *Cast<SphereShapeGeometryActor>(volatileActor.get());
		auto& renderComponent = *sphereActor.getSphereShapeGeometryComponent();
		renderComponent.setInstanceMaterial(materialQuery.value);

	}

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

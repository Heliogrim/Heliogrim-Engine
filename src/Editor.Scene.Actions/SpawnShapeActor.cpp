#include "SpawnShapeActor.hpp"

using namespace hg::editor::scene::actions;
using namespace hg;

/**/

[[nodiscard]] static Opt<ActorRef<>> spawnActorImpl(
	SpawnShapeActor::ShapeType shapeType_,
	ref<const math::Location> location_
);

static void despawnActorImpl(cref<ActorRef<>> actorRef_);

/**/

SpawnShapeActor::SpawnShapeActor(ShapeType shapeType_, mref<math::Location> location_) :
	DelegateAction(
		[this, shapeType_, location_] {
			_actorRef = spawnActorImpl(shapeType_, location_);
		},
		[this] {
			if (_actorRef != None) {
				despawnActorImpl(*std::exchange(_actorRef, None));
			}
		}
	) {}

SpawnShapeActor::~SpawnShapeActor() = default;

/**/

#include <Editor.Assets.Default/GraphicsMaterials/Default.hpp>
#include <Editor.Core/EditorEngine.hpp>
#include <Editor.UI.Main/EditorUI.hpp>
#include <Editor.UI.Main/Module/EditorUI.hpp>
#include <Engine.Common/Discard.hpp>
#include <Engine.Common/Move.hpp>
#include <Heliogrim/Heliogrim.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Actor/Geometry/BoxShapeGeometryActor.hpp>
#include <Heliogrim/Actor/Geometry/SphereShapeGeometryActor.hpp>
#include <Heliogrim/Asset/MaterialAssetHandles.hpp>
#include <Heliogrim/Core/Session.hpp>
#include <Heliogrim/Universe/Universe.hpp>

Opt<ActorRef<>> spawnActorImpl(SpawnShapeActor::ShapeType shapeType_, ref<const math::Location> location_) {

	/* Generate actor */

	auto session = GetSession();
	auto universe = GetUniverse(session);

	const auto materialQuery = Heliogrim::assets().find<GraphicsMaterialAssetHandle>(editor::assets::materials::default_brdf_guid);
	::hg::assertrt(materialQuery.flags & AssetDatabaseResultType::eSuccess);

	auto volatileActor = [&]() -> VolatileActor<> {
		if (shapeType_ == SpawnShapeActor::ShapeType::eBox) {
			auto boxActor = CreateActor<BoxShapeGeometryActor>();
			boxActor->get<"geometry">().setInstanceMaterial(materialQuery.value);
			return VolatileActor<> { ::hg::move(boxActor) };

		} else if (shapeType_ == SpawnShapeActor::ShapeType::eSphere) {
			auto sphereActor = CreateActor<SphereShapeGeometryActor>();
			sphereActor->get<"geometry">().setInstanceMaterial(materialQuery.value);
			return VolatileActor<> { ::hg::move(sphereActor) };

		} else {
			std::unreachable();
		}
	}();

	/**/

	volatileActor->getRootComponent()->getLocalTransform().location() = location_;
	auto actorRef = universe.addActor(::hg::move(volatileActor));

	/**/

	auto* const editorUi = static_cast<editor::EditorUI*>(
		editor::EditorEngine::getEngine()->getModules().getSubModule(editor::EditorUIDepKey).get()
	);
	editorUi->getEditorModel().scene.instance.next(universe.unwrap().get());

	/**/

	return actorRef;
}

void despawnActorImpl(cref<ActorRef<>> actorRef_) {

	/* Generate actor */

	auto session = GetSession();
	auto universe = GetUniverse(session);

	auto actor = actorRef_.get();
	if (actor == None) {
		return;
	}
	universe.dropActor(std::addressof(actor.value()));

	/**/

	auto* const editorUi = static_cast<editor::EditorUI*>(
		editor::EditorEngine::getEngine()->getModules().getSubModule(editor::EditorUIDepKey).get()
	);
	editorUi->getEditorModel().scene.instance.next(universe.unwrap().get());
}

#include "SpawnLightActor.hpp"

#include <Editor.Core/EditorEngine.hpp>
#include <Editor.UI.Main/EditorUI.hpp>
#include <Editor.UI.Main/Module/EditorUI.hpp>
#include <Engine.ACS/ActorModule.hpp>
#include <Engine.Common/Discard.hpp>
#include <Engine.Core/Engine.hpp>
#include <Heliogrim/ActorInitializer.hpp>
#include <Heliogrim/Component/Scene/Light/DirectionalLightComponent.hpp>
#include <Heliogrim/Core/Session.hpp>
#include <Heliogrim/Universe/Universe.hpp>

using namespace hg::editor::scene::actions;
using namespace hg;

/**/

[[nodiscard]] static Opt<ActorGuid> spawnLightActorImpl(
	ref<const math::Location> location_,
	ref<const SpawnLightActorOptions> options_
);

static void despawnLightActorImpl(mref<ActorGuid> actorGuid_);

/**/

SpawnLightActor::SpawnLightActor(mref<math::Location> location_, mref<SpawnLightActorOptions> options_) :
	DelegateAction(
		[this, location_, options_] {
			_actorGuid = spawnLightActorImpl(location_, options_);
		},
		[this] {
			if (_actorGuid != None) {
				despawnLightActorImpl(*std::exchange(_actorGuid, None));
			}
		}
	) {}

SpawnLightActor::~SpawnLightActor() = default;

/**/

Opt<ActorGuid> spawnLightActorImpl(ref<const math::Location> location_, ref<const SpawnLightActorOptions> options_) {

	::hg::assertrt(options_.is<SpawnDirectionalLightActorOptions>());
	const auto& options = options_.as<SpawnDirectionalLightActorOptions>();

	/* Generate Actor */

	auto session = GetSession();
	auto universe = GetUniverse(session);

	auto volatileActor = CreateActor();
	auto component = volatileActor->addComponent<DirectionalLightComponent>();

	/* Configure Components */

	component->_luminance = options.luminance;
	component->_direction = options.direction;

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

void despawnLightActorImpl(mref<ActorGuid> actorGuid_) {

	/* Generate Actor */

	auto session = GetSession();
	auto universe = GetUniverse(session);

	auto* actor = engine::Engine::getEngine()->getActors()->getRegistry()->getOrCreateActorPool<Actor>()->get(actorGuid_);
	::hg::discard = Destroy(::hg::move(actor), universe);

	/**/

	auto* const editorUi = static_cast<editor::EditorUI*>(
		editor::EditorEngine::getEngine()->getModules().getSubModule(editor::EditorUIDepKey).get()
	);
	editorUi->getEditorModel().scene.instance.next(universe.unwrap().get());
}

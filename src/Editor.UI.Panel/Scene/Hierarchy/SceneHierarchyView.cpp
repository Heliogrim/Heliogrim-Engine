#include "SceneHierarchyView.hpp"

#include <functional>
#include <Editor.Action/ActionManager.hpp>
#include <Editor.Assets.Default/Meshes/Default.hpp>
#include <Editor.Scene.Actions/SpawnGeometryActor.hpp>
#include <Editor.Scene.Actions/SpawnLightActor.hpp>
#include <Editor.UI/Builder/MenuBuilder.hpp>
#include <Editor.UI/Widget/ContextMenuProvider.hpp>
#include <Editor.UI.Main/EditorUI.hpp>
#include <Editor.UI.Service/Scene/Hierarchy/SceneHierarchyEntry.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.Reflow.Uikit/Atom/Paint.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/HorizontalLayout.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Component/Scene/Collision/BoxShapeCollisionComponent.hpp>
#include <Heliogrim/Component/Scene/Collision/SphereShapeCollisionComponent.hpp>

#include "SceneHierarchyController.hpp"
#include "SceneHierarchyModel.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

/**/

SceneHierarchyView::SceneHierarchyView(
	ref<SceneHierarchyController> controller_,
	SharedPtr<uikit::Card> main_,
	std::weak_ptr<TreeView<service::SceneHierarchyEntry, std::type_identity_t>> tree_
) :
	_controller(controller_),
	main(::hg::move(main_)),
	tree(::hg::move(tree_)) {}

SharedPtr<Widget> SceneHierarchyView::makeTreeItem(ref<const service::SceneHierarchyEntry> data_) const {
	switch (data_.type()) {
		case service::SceneHierarchyEntryType::eActor: {

			const auto actor = data_.value.as<nmpt<Actor>>();
			const auto guid = actor->guid();

			const auto text = make_sptr<uikit::Text>();

			if (actor->label().text.empty()) {
				text->setText(std::format(R"(Actor << {}-{}-{}-{} >>)", guid.pre, guid.c0, guid.c1, guid.post));
			} else {
				text->setText(actor->label().text);
			}

			auto ctxMenu = make_sptr<ContextMenuProvider>();
			ctxMenu->setContent(::hg::move(text));
			ctxMenu->onContextMenu(
				[actor](ref<const ContextMenuEvent> event_) {
					generateActorActions(event_.builder, actor);
					return EventResponse::eHandled;
				}
			);

			return ctxMenu;
		}
		case service::SceneHierarchyEntryType::eComponent: {

			const auto component = data_.value.as<nmpt<HierarchyComponent>>();
			const auto typeId = component->getTypeId();

			const auto text = make_sptr<uikit::Text>();
			if (component->label().text.empty()) {
				text->setText(std::format(R"(Component << {} >>)", typeId.data));
			} else {
				text->setText(component->label().text);
			}

			return text;

			auto ctxMenu = make_sptr<ContextMenuProvider>();
			ctxMenu->setContent(::hg::move(text));
			ctxMenu->onContextMenu(
				[component](ref<const ContextMenuEvent> event_) {
					generateComponentActions(event_.builder, component);
					return EventResponse::eHandled;
				}
			);

			return ctxMenu;
		}
		default: return nullptr;
	}
}

void SceneHierarchyView::displayTreeItems(
	ref<const Vector<service::SceneHierarchyEntry>> baseItems_,
	mref<std::function<void(ref<const service::SceneHierarchyEntry> parent_, ref<Vector<service::SceneHierarchyEntry>> children_)>>
	resolver_
) {

	if (tree.expired()) {
		return;
	}

	auto locked = tree.lock();
	locked->_generateFromData = [this](const auto& data_) { return makeTreeItem(data_); };
	locked->_resolveChildren = ::hg::move(resolver_);
	locked->_selectedFnc = [this](const auto& selectedSet_) {

		if (selectedSet_.empty()) {
			_controller.onSelectionChange({});
			return;
		}

		auto tmp = Vector<service::SceneHierarchyEntry> {};
		tmp.reserve(selectedSet_.size());
		for (auto&& selected : selectedSet_) { tmp.emplace_back(::hg::move(selected)); }
		_controller.onSelectionChange(tmp);
	};

	locked->setTreeViewSource(baseItems_);
}

/**/

void generateSceneActions(
	ref<MenuBuilder> builder_,
	ref<const Variant<nmpt<engine::core::Universe>, nmpt<engine::core::Level>, nmpt<Actor>>> scene_
) {
	auto subjectTitle = StringView { "Universe"sv };
	if (scene_.is<nmpt<engine::core::Level>>()) {
		subjectTitle = "Level"sv;
	} else if (scene_.is<nmpt<Actor>>()) {
		subjectTitle = "Root Actor"sv;
	}

	auto addItem = builder_.addItem("AddToScene"sv);
	addItem.setPrecedence(20u);
	addItem.setTitle(std::format(R"(Add to {})", subjectTitle));

	auto subAddItemSearch = addItem.addSubItem("AddToScene::Search"sv);
	subAddItemSearch.setTitle("Search"sv);
	auto subAddItemEmpty = addItem.addSubItem("AddToScene::Empty"sv);
	subAddItemEmpty.setTitle("Empty"sv);
	auto subAddItemGeometry = addItem.addSubItem("AddToScene::Geometry"sv);
	subAddItemGeometry.setTitle("Geometry"sv);
	auto subAddItemEffect = addItem.addSubItem("AddToScene::Effect"sv);
	subAddItemEffect.setTitle("Effect"sv);
	auto subAddItemLight = addItem.addSubItem("AddToScene::Light"sv);
	subAddItemLight.setTitle("Light"sv);
	auto subAddItemAudio = addItem.addSubItem("AddToScene::Audio"sv);
	subAddItemAudio.setTitle("Audio"sv);

	/**/

	auto addCubeItem = subAddItemGeometry.addSubItem("AddToScene::Geometry::Cube"sv);
	addCubeItem.setTitle("Cube"sv);
	addCubeItem.setAction(
		[] {
			const auto module = engine::Engine::getEngine()->getModules().getSubModule(editor::ActionDepKey);
			::hg::assertrt(module != nullptr);

			static_cast<ptr<editor::ActionManager>>(module.get())->apply(
				Arci<editor::scene::actions::SpawnGeometryActor>::create(**editor::assets::meshes::get_default_cube(), math::Location {})
			);
			return true;
		}
	);

	auto addSphereItem = subAddItemGeometry.addSubItem("AddToScene::Geometry::Sphere"sv);
	addSphereItem.setTitle("Sphere"sv);
	addSphereItem.setAction(
		[] {
			const auto module = engine::Engine::getEngine()->getModules().getSubModule(editor::ActionDepKey);
			::hg::assertrt(module != nullptr);

			static_cast<ptr<editor::ActionManager>>(module.get())->apply(
				Arci<editor::scene::actions::SpawnGeometryActor>::create(**editor::assets::meshes::get_default_sphere(), math::Location {})
			);
			return true;
		}
	);

	/**/

	auto addDirLightItem = subAddItemLight.addSubItem("AddToScene::Light::Directional"sv);
	addDirLightItem.setTitle("Directional Light"sv);
	addDirLightItem.setAction(
		[] {
			const auto module = engine::Engine::getEngine()->getModules().getSubModule(editor::ActionDepKey);
			::hg::assertrt(module != nullptr);

			static_cast<ptr<editor::ActionManager>>(module.get())->apply(
				Arci<editor::scene::actions::SpawnLightActor>::create(
					math::Location {},
					editor::scene::actions::SpawnDirectionalLightActorOptions {}
				)
			);
			return true;
		}
	);
}

void generateActorActions(ref<MenuBuilder> builder_, nmpt<Actor> actor_) {
	auto addItem = builder_.addItem("AddToActor"sv);
	addItem.setPrecedence(10u);
	addItem.setTitle("Add to Actor"sv);

	auto subAddItemPhysics = addItem.addSubItem("AddToActor::Physics"sv);
	subAddItemPhysics.setTitle("Physics"sv);

	auto addBoxShapeCollision = subAddItemPhysics.addSubItem("AddToActor::Physics::BoxShapeCollision");
	addBoxShapeCollision.setTitle("Box Shape Collision"sv);
	addBoxShapeCollision.setAction(
		[actor_] {
			auto root = actor_->getRootComponent();
			actor_->addComponent<BoxShapeCollisionComponent>({ .autoRegister = true, .parent = root });
			return true;
		}
	);

	auto addSphereShapeCollision = subAddItemPhysics.addSubItem("AddToActor::Physics::SphereShapeCollision");
	addSphereShapeCollision.setTitle("Sphere Shape Collision"sv);
	addSphereShapeCollision.setAction(
		[actor_] {
			auto root = actor_->getRootComponent();
			actor_->addComponent<SphereShapeCollisionComponent>({ .autoRegister = true, .parent = root });
			return true;
		}
	);

	auto copyItem = builder_.addItem("CopyActor"sv);
	copyItem.setTitle("Copy"sv);

	auto pasteItem = builder_.addItem("PasteToActor"sv);
	pasteItem.setTitle("Paste"sv);

	auto renameItem = builder_.addItem("RenameActor"sv);
	renameItem.setTitle("Rename"sv);

	auto duplicateItem = builder_.addItem("DuplicateActor"sv);
	duplicateItem.setTitle("Duplicate"sv);

	/**/

	auto saveAsItem = builder_.addItem("SaveActorAs"sv);
	saveAsItem.setTitle("Save As"sv);
	saveAsItem.setSpacer(true, false);

	/**/

	auto cutItem = builder_.addItem("CutActor"sv);
	cutItem.setTitle("Cut"sv);
	cutItem.setSpacer(true, false);

	auto deleteItem = builder_.addItem("RemoveActor"sv);
	deleteItem.setTitle("Remove");
	deleteItem.setAction(
		[actor_]() {
			const auto& sceneInstance = editor::EditorUI::getEditorUI()->getEditorModel().scene.instance.peek();
			::hg::assertd(not sceneInstance.is<nmpt<Actor>>());

			const auto& sceneSelected = editor::EditorUI::getEditorUI()->getEditorModel().scene.selected.peek();
			if (sceneSelected.is<nmpt<Actor>>() && sceneSelected.as<nmpt<Actor>>() == actor_) {
				editor::EditorUI::getEditorUI()->getEditorModel().scene.selected.next(nullptr);
			}

			if (sceneInstance.is<nmpt<engine::core::Universe>>()) {

				const auto universe = sceneInstance.as<nmpt<engine::core::Universe>>();
				if (universe != nullptr && actor_->getUniverse().address() == universe.get()) {
					actor_->getLevel()->removeActor(actor_).reset();
				}
				return true;
			}

			if (sceneInstance.is<nmpt<engine::core::Level>>()) {

				const auto level = sceneInstance.as<nmpt<engine::core::Level>>();
				if (level != nullptr && actor_->getLevel().address() == level.get()) {
					level->removeActor(actor_).reset();
				}
				return true;
			}

			return true;
		}
	);
}

void generateComponentActions(ref<MenuBuilder> builder_, nmpt<HierarchyComponent> component_) {

	auto addItem = builder_.addItem("AddToComponent"sv);

	auto subAddItemPhysics = addItem.addSubItem("AddToComponent::Physics"sv);
	auto addBoxShapeCollision = subAddItemPhysics.addSubItem("AddToComponent::Physics::BoxShapeCollision"sv);
	addBoxShapeCollision.setAction(
		[component_] mutable {
			const auto actor = component_->getOwner();
			actor->addComponent<BoxShapeCollisionComponent>({ .autoRegister = true, .parent = Some(*component_) });
			return true;
		}
	);

	auto duplicateItem = builder_.addItem("DuplicateComponent"sv);
	duplicateItem.setTitle("Duplicate"sv);

	auto deleteItem = builder_.addItem("RemoveComponent"sv);
	deleteItem.setTitle("Remove"sv);
}

UniquePtr<SceneHierarchyView> editor::ui::makeSceneHierarchyView(ref<SceneHierarchyController> controller_) {

	auto header = make_sptr<uikit::HorizontalLayout>();

	/**/

	const auto contentStack = make_sptr<uikit::Stack>();
	const auto contentBg = make_sptr<uikit::Paint>(ReflowClassList { "[Test]"sv }, nullptr);

	//const auto content = make_sptr<uikit::VScrollBox>();
	const auto tree = make_sptr<TreeView<service::SceneHierarchyEntry, std::type_identity_t>>();

	contentStack->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
	contentStack->getLayoutAttributes().update<attr::BoxLayout::heightGrow>(1.F);
	contentStack->getLayoutAttributes().update<attr::BoxLayout::padding>(Padding { 8.F });
	(std::get<0>(tree->getLayoutAttributes().attributeSets)).update<attr::BoxLayout::widthGrow>(1.F);
	(std::get<0>(tree->getLayoutAttributes().attributeSets)).update<attr::BoxLayout::heightGrow>(1.F);

	//content->addChild(clone(tree));
	contentStack->addChild(::hg::move(contentBg));
	contentStack->addChild(clone(tree));

	/**/

	auto wrapper = make_sptr<uikit::VerticalLayout>();

	(std::get<0>(wrapper->getLayoutAttributes().attributeSets)).update<attr::BoxLayout::widthGrow>(1.F);
	(std::get<0>(wrapper->getLayoutAttributes().attributeSets)).update<attr::BoxLayout::heightGrow>(1.F);

	wrapper->addChild(::hg::move(header));
	wrapper->addChild(::hg::move(contentStack));

	auto ctxMenu = make_sptr<ContextMenuProvider>();
	ctxMenu->setContent(::hg::move(wrapper));
	ctxMenu->onContextMenu(
		[](ref<const ContextMenuEvent> event_) {
			generateSceneActions(event_.builder, EditorUI::getEditorUI()->getEditorModel().scene.instance.peek());
			return EventResponse::eHandled;
		}
	);

	auto card = uikit::makeCard({ .level = 1, .children = ::hg::move(ctxMenu) });

	return make_uptr<SceneHierarchyView>(
		controller_,
		::hg::move(card),
		tree
	);
}

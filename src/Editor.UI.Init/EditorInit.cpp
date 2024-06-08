#include "EditorInit.hpp"

#include <Editor.Core/EditorEngine.hpp>
#include <Editor.Core/HeliogrimEditor.hpp>
#include <Editor.GFX.Graphs/Node/Input/SubGraphInputNode.hpp>
#include <Editor.GFX.Graphs/Node/Math/ScalarPowMathNode.hpp>
#include <Editor.GFX.Graphs/Node/Math/ScalarSubMathNode.hpp>
#include <Editor.GFX.Graphs/Node/Output/SubGraphOutputNode.hpp>
#include <Editor.UI/Color/Dark.hpp>
#include <Editor.UI/Modules/AssetBrowser.hpp>
#include <Editor.UI/Modules/ObjectEditor.hpp>
#include <Editor.UI/Modules/SceneHierarchy.hpp>
#include <Editor.UI/Panel/AssetBrowserPanel.hpp>
#include <Editor.UI/Panel/ObjectEditorPanel.hpp>
#include <Editor.UI/Panel/SceneHierarchyPanel.hpp>
#include <Editor.UI/Widget/Board/Board.hpp>
#include <Editor.UI/Widget/Board/BoardNode.hpp>
#include <Editor.UI/Widget/Board/Whiteboard.hpp>
#include <Editor.UI.Main/EditorUI.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/World.hpp>
#include <Engine.Core/WorldContext.hpp>
#include <Engine.Core/Event/SignalShutdownEvent.hpp>
#include <Engine.GFX/Device/Device.hpp>
#include <Engine.GFX/Swapchain/VkSwapchain.hpp>
#include <Engine.GFX.Scene/RenderSceneManager.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Reflow/Padding.hpp>
#include <Engine.Reflow/ReflowSpacing.hpp>
#include <Engine.Reflow/ReflowUnit.hpp>
#include <Engine.Reflow/Widget/Button.hpp>
#include <Engine.Reflow/Widget/Image.hpp>
#include <Engine.Reflow/Widget/Menu.hpp>
#include <Engine.Reflow/Widget/Overlay.hpp>
#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Reflow/Widget/Viewport.hpp>
#include <Engine.Reflow/Window/Window.hpp>
#include <Engine.Reflow/Window/WindowManager.hpp>
#include <Engine.Resource/ResourceUsageFlag.hpp>
#include <Engine.Scheduler/Async.hpp>
#include <Heliogrim/Actor.hpp>
#include <Heliogrim/UIComponent.hpp>
#include <Heliogrim/World.hpp>
#include <Heliogrim/Actors/CameraActor.hpp>

using namespace hg::editor::ui;
using namespace hg;

/**/

static void initDefaultUi(
	ref<editor::EditorUI> editorUi_,
	cref<sptr<engine::gfx::Device>> gfxDevice_,
	nmpt<engine::reflow::Window> window_
);

/**/

void editor::ui::initEditor(ref<EditorUI> editorUi_) {

	const auto& engine = *EditorEngine::getEngine();
	const auto editorSession = engine.getEditorSession();
	const auto editorWorld = editorSession->getWorldContext().getCurrentWorld();

	/**/

	const auto window = engine::reflow::WindowManager::get()->requestWindow(
		"Heliogrim Engine"sv,
		math::ivec2 { 1280, 720 },
		{},
		"Editor-Renderer"sv,
		editorWorld->getScene()
	);

	initDefaultUi(editorUi_, engine.getGraphics()->getCurrentDevice(), window.get());

	/**/

	auto* const actor = CreateActor(HeliogrimEditor::getEditorSession());
	auto* const uic = HeliogrimEditor::getEditorSession().getActorInitializer().createComponent<UIComponent>(actor);

	uic->setWindow(window);
	GetWorld(HeliogrimEditor::getEditorSession()).addActor(actor);

}

/**/

#pragma region Temporary

using namespace hg::engine::reflow;

static void loadActorMappingExp(
	ref<editor::EditorUI> editorUI_,
	const type_id typeId_,
	cref<sptr<ObjectEditorPanel>> panel_
) {
	if (not editorUI_.editorSelectedTarget) {
		__debugbreak();
		return;
	}

	panel_->setEditorTarget(typeId_, editorUI_.editorSelectedTarget.get());
}

void editor::ui::storeEditorSelectedTarget(ref<EditorUI> editorUI_, const hg::ptr<hg::Actor> target_) {
	if (not editorUI_.getObjectEditor()) {
		return;
	}

	const auto& panels { editorUI_.getObjectEditor()->_panels };
	if (panels.empty()) {
		return;
	}

	const auto panel { panels.front().lock() };
	editorUI_.editorSelectedTarget = target_;
	loadActorMappingExp(editorUI_, "hg::Actor"_typeId, panel);
}

static void storeHierarchyActor(ref<editor::EditorUI> editorUI_, cref<Vector<ptr<Actor>>> targets_) {
	if (not editorUI_.getSceneHierarchy()) {
		return;
	}

	const auto& panels { editorUI_.getSceneHierarchy()->_panels };
	if (panels.empty()) {
		return;
	}

	const auto panel { panels.front().lock() };

	Vector<sptr<SceneViewEntry>> sources {};

	for (const auto& actor : targets_) {
		auto sve { make_sptr<SceneViewEntry>() };
		sve->storeTarget<Actor>(actor);
		sources.push_back(std::move(sve));
	}

	panel->setHierarchyTarget<SceneViewEntry>("sptr<SceneViewEntry>"_typeId, sources);
}

void editor::ui::storeEditorSelectedTarget(ref<EditorUI> editorUI_, const hg::ptr<hg::ActorComponent> target_) {
	if (not editorUI_.getObjectEditor()) {
		return;
	}

	const auto& panels { editorUI_.getObjectEditor()->_panels };
	if (panels.empty()) {
		return;
	}

	const auto panel { panels.front().lock() };
	editorUI_.editorSelectedTarget = target_;
	loadActorMappingExp(editorUI_, target_->getTypeId(), panel);
}

static void configureMainViewport(
	ref<editor::EditorUI> editorUi_,
	sptr<VerticalPanel> parent_
) {

	const auto defaultFont = editorUi_.getDefaultFont();

	/**/

	auto viewportCtrls = make_sptr<HorizontalPanel>();
	viewportCtrls->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	viewportCtrls->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	viewportCtrls->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 24.F });
	viewportCtrls->attr.height.setValue({ ReflowUnitType::eAuto });
	viewportCtrls->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
	viewportCtrls->attr.padding.setValue(Padding { 4.F, 4.F });
	viewportCtrls->attr.justify.setValue(ReflowSpacing::eSpaceAround);
	viewportCtrls->attr.colGap.setValue(8.F);
	viewportCtrls->attr.flexGrow.setValue(1.F);

	auto viewportWrapper = make_sptr<HorizontalPanel>();
	viewportWrapper->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	viewportWrapper->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	viewportWrapper->attr.height.setValue({ ReflowUnitType::eRelative, 1.F });
	viewportWrapper->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
	viewportWrapper->attr.justify.setValue(ReflowSpacing::eSpaceAround);
	viewportWrapper->attr.flexGrow.setValue(1.F);
	viewportWrapper->attr.flexShrink.setValue(1.F);

	parent_->addChild(viewportCtrls);
	parent_->addChild(viewportWrapper);

	/**/

	auto playButton = make_sptr<Button>();
	playButton->attributes().padding.setValue(Padding { 8.F, 4.F });

	auto pauseButton = make_sptr<Button>();
	pauseButton->attributes().padding.setValue(Padding { 8.F, 4.F });

	auto stopButton = make_sptr<Button>();
	stopButton->attributes().padding.setValue(Padding { 8.F, 4.F });

	auto playText = make_sptr<Text>();
	playText->attr.font.setValue(*defaultFont);
	playText->attr.textAlign.setValue(TextAlign::eCenterMiddle);
	playText->attr.color.setValue(color::Dark::white);
	playText->setText("Play");

	auto pauseText = make_sptr<Text>();
	pauseText->attr.font.setValue(*defaultFont);
	pauseText->attr.textAlign.setValue(TextAlign::eCenterMiddle);
	pauseText->attr.color.setValue(color::Dark::white);
	pauseText->setText("Pause");

	auto stopText = make_sptr<Text>();
	stopText->attr.font.setValue(*defaultFont);
	stopText->attr.textAlign.setValue(TextAlign::eCenterMiddle);
	stopText->attr.color.setValue(color::Dark::white);
	stopText->setText("Stop");

	viewportCtrls->addChild(playButton);
	viewportCtrls->addChild(pauseButton);
	viewportCtrls->addChild(stopButton);

	playButton->setChild(playText);
	pauseButton->setChild(pauseText);
	stopButton->setChild(stopText);

	/**
	 *
	 */
	auto viewport = make_sptr<Viewport>();
	viewport->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	viewport->attr.height.setValue({ ReflowUnitType::eRelative, 1.F });

	viewportWrapper->addChild(viewport);

	/**/

	const auto gfx { editor::EditorEngine::getEngine()->getGraphics() };
	const auto coreSession = editor::EditorEngine::getEngine()->getPrimaryGameSession();
	const auto coreWorld { coreSession->getWorldContext().getCurrentWorld() };
	const auto scene { coreWorld->getScene() };

	//RegisterActorClass<CameraActor>();
	coreSession->getState().getRegistry().getOrCreateActorPool<CameraActor>();

	auto session = HeliogrimEditor::getSession();
	ptr<CameraActor> camera { CreateActor<CameraActor>(session) };
	auto world = GetWorld(session); {
		cref<math::Transform> tf = camera->getRootComponent()->getWorldTransform();
		const_cast<ref<math::Transform>>(tf).setLocation(math::Location { 0.F, 0.F, -5.F });
		//const_cast<ref<math::Transform>>(tf).setLocation(math::Location { 0.F, 1.8F, 0.F });
	}
	world.addActor(camera);

	/**/

	viewport->setViewportTarget("Test-Renderer"sv, world, camera);
	viewport->rebuildView();

	/**/

	/**/

	viewport->addResizeListener(
		[instance = viewport.get()](mref<smr<engine::gfx::Swapchain>> prev_, mref<smr<engine::gfx::Swapchain>> next_) {

			if (not instance->hasMountedRenderTarget()) {
				return;
			}

			auto* const manager = engine::Engine::getEngine()->getGraphics()->getSceneManager();
			manager->transitionToTarget(std::move(prev_), std::move(next_), nullptr);
		}
	);

	/**/

	auto viewportOverlay = make_sptr<Overlay>();
	auto viewOverBox = make_sptr<HorizontalPanel>(); {
		auto& attr = viewOverBox->attr;
		attr.width.setValue(ReflowUnitType::eAuto, 0.F);
		attr.height.setValue(ReflowUnitType::eAuto, 0.F);
		attr.style.setValue(
			{
				.backgroundColor = engine::color { 1.F, 1.F, 1.F, 0.F }
			}
		);
	}

	viewportWrapper->addChild(viewportOverlay);
	viewportOverlay->setContent(viewOverBox); {
		auto statsWrapper = make_sptr<VerticalPanel>(); {
			auto& attr = statsWrapper->attr;
			attr.width.setValue(ReflowUnitType::eAuto, 0.F);
			attr.height.setValue(ReflowUnitType::eAuto, 0.F);
			attr.style.setValue(
				{
					.backgroundColor = engine::color { 1.F, 1.F, 1.F, 0.F }
				}
			);
		}

		viewOverBox->addChild(statsWrapper);

		/**/

		auto sceneName = make_sptr<Text>();
		sceneName->attr.font.setValue(*defaultFont);
		sceneName->attr.fontSize.setValue(16.F);
		sceneName->attr.color.setValue(engine::color { 229.F, 190.F, 1.F, 255.F });
		sceneName->setText("Test Scene");

		statsWrapper->addChild(sceneName);

		auto sceneCalcTime = make_sptr<Text>();
		sceneCalcTime->attr.font.setValue(*defaultFont);
		sceneCalcTime->attr.fontSize.setValue(16.F);
		sceneCalcTime->attr.color.setValue(engine::color { 229.F, 190.F, 1.F, 255.F });
		sceneCalcTime->setText("<$ftime> ms");

		statsWrapper->addChild(sceneCalcTime);

		auto sceneFrames = make_sptr<Text>();
		sceneFrames->attr.font.setValue(*defaultFont);
		sceneFrames->attr.fontSize.setValue(16.F);
		sceneFrames->attr.color.setValue(engine::color { 229.F, 190.F, 1.F, 255.F });
		sceneFrames->setText("<$frames> FPS");

		statsWrapper->addChild(sceneFrames);
	}

}

static void configureMainGraph(
	sptr<VerticalPanel> parent_
) {

	auto whiteboard = make_sptr<Whiteboard>();
	auto board = make_sptr<Board>();

	whiteboard->setBoard(board);
	parent_->addChild(whiteboard);

	/**/

	auto node0 = make_sptr<editor::gfx::graph::nodes::SubGraphInputNode>();
	node0->setName("Out Input Node");
	node0->regenerate();
	auto test0 = BoardNode::make(node0);

	auto node1 = make_sptr<editor::gfx::graph::nodes::ScalarSubMathNode>();
	node1->setName("Useless Operation #1");
	node1->regenerate();
	auto test1 = BoardNode::make(node1);

	auto node2 = make_sptr<editor::gfx::graph::nodes::ScalarPowMathNode>();
	node2->setName("Useless Operation #2");
	node2->regenerate();
	auto test2 = BoardNode::make(node2);

	auto node3 = make_sptr<editor::gfx::graph::nodes::SubGraphOutputNode>();
	node3->setName("Out Output Node");
	node3->regenerate();
	auto test3 = BoardNode::make(node3);

	test0->setBoardPosition(math::vec2 { -240.F, 0.F });
	//test0->style()->margin.attr.x = -240.F;
	test1->setBoardPosition(math::vec2 { 0.F, -50.F });
	//test1->style()->margin.attr.y = -50.F;
	test2->setBoardPosition(math::vec2 { 0.F, 50.F });
	//test2->style()->margin.attr.y = 50.F;
	test3->setBoardPosition(math::vec2 { 240.F, 0.F });
	//test3->style()->margin.attr.x = 240.F;

	board->addChild(test0);
	board->addChild(test1);
	board->addChild(test2);
	board->addChild(test3);
}

void initDefaultUi(
	ref<editor::EditorUI> editorUi_,
	cref<sptr<engine::gfx::Device>> gfxDevice_,
	nmpt<engine::reflow::Window> window_
) {

	/**/
	constexpr math::vec2 available { 1280.F, 720.F };
	constexpr math::vec2 shift { 0.F };

	FlowContext ctx {
		math::fExtent2D { 1280.F, 720.F, 0.F, 0.F },
		math::fExtent2D { 1280.F, 720.F, 0.F, 0.F },
	};

	/**/
	auto root = make_sptr<VerticalPanel>();
	root->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	root->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	root->attr.height.setValue({ ReflowUnitType::eRelative, 1.F });
	root->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });

	const auto defaultFont = editorUi_.getDefaultFont();

	/**/

	/**
	 *
	 */
	auto navSection = make_sptr<HorizontalPanel>();
	navSection->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	navSection->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	navSection->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 24.F });
	navSection->attr.height.setValue({ ReflowUnitType::eAuto });
	navSection->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
	navSection->attr.colGap.setValue(4.F);
	navSection->attr.padding.setValue(Padding { 2.F, 6.F });
	navSection->attr.flexGrow.setValue(1.F);

	auto contentSection = make_sptr<HorizontalPanel>();
	contentSection->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	contentSection->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	contentSection->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 24.F });
	contentSection->attr.height.setValue({ ReflowUnitType::eRelative, 1.F });
	contentSection->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
	contentSection->attr.justify.setValue(ReflowSpacing::eSpaceEvenly);
	contentSection->attr.flexGrow.setValue(1.F);
	contentSection->attr.flexShrink.setValue(1.F);

	root->addChild(navSection);
	root->addChild(contentSection);

	/**
	 *
	 */

	auto navBrandIcon = make_sptr<Image>();
	ReflowUnit abs32 { ReflowUnitType::eAbsolute, 32.F };
	navBrandIcon->attr.minWidth.setValue(abs32);
	navBrandIcon->attr.width.setValue(abs32);
	navBrandIcon->attr.maxWidth.setValue(abs32);
	navBrandIcon->attr.minHeight.setValue(abs32);
	navBrandIcon->attr.height.setValue(abs32);
	navBrandIcon->attr.maxHeight.setValue(abs32);

	// TODO: This will break
	auto testGuard = editorUi_.getPlaceholderImage()->acquire(engine::resource::ResourceUsageFlag::eAll);
	navBrandIcon->setImage(*testGuard.imm(), nullptr);

	auto navFileButton = make_sptr<Button>();
	navFileButton->attributes().padding.setValue(Padding { 4.F, 2.F });
	//navFileButton->attr.color.setValue(color::Dark::darkRed);

	auto navFileText = make_sptr<Text>();
	navFileText->attr.font.setValue(*defaultFont);
	navFileText->attr.fontSize.setValue(16.F);
	navFileText->attr.color.setValue(color::Dark::grey);

	navFileButton->setChild(navFileText);
	navFileText->setText("File");

	auto navSaveAllButton = make_sptr<Button>();
	navSaveAllButton->attributes().padding.setValue(Padding { 4.F, 2.F });
	//navFileButton->attr.color.setValue(color::Dark::backgroundDefault);

	auto navSaveAllText = make_sptr<Text>();
	navSaveAllText->attr.font.setValue(*defaultFont);
	navSaveAllText->attr.fontSize.setValue(16.F);
	navSaveAllText->attr.color.setValue(color::Dark::grey);

	navSaveAllButton->setChild(navSaveAllText);
	navSaveAllText->setText("Save All");

	auto navQuitButton = make_sptr<Button>();
	navQuitButton->attributes().padding.setValue(Padding { 4.F, 2.F });
	//navQuitButton->attr.color.setValue(color::Dark::backgroundDefault);

	auto navQuitText = make_sptr<Text>();
	navQuitText->attr.font.setValue(*defaultFont);
	navQuitText->attr.fontSize.setValue(16.F);
	navQuitText->attr.color.setValue(color::Dark::grey);

	navQuitButton->setChild(navQuitText);
	navQuitText->setText("Quit");

	[[maybe_unused]] auto qblid = navQuitButton->addOnClick(
		[](cref<engine::input::event::MouseButtonEvent> event_) {
			if (not event_._down)
				return;
			::hg::engine::scheduler::exec(
				[] {
					engine::Engine::getEngine()->getEmitter().emit(engine::core::SignalShutdownEvent {});
				}
			);
		}
	);

	auto navFileMenu = make_sptr<Menu>();
	auto nfmw = make_sptr<VerticalPanel>();

	navFileMenu->setContent(nfmw);
	navFileMenu->closeMenu();

	//root->addChild(navFileMenu);

	[[maybe_unused]] auto handle = navFileButton->addOnClick(
		[self = wptr<Widget> { navFileButton }, nfm = wptr<Menu> { navFileMenu }](auto event_) {
			if (not event_._down || nfm.expired()) {
				return;
			}

			auto lck { nfm.lock() };
			if (lck) {
				lck->openMenu();

				FocusEvent focusEvent { lck };
				auto root { self.lock()->root() };

				WindowManager::get()->dispatch(std::static_pointer_cast<Window, Widget>(root), focusEvent);
			}
		}
	);

	nfmw->addChild(navSaveAllButton);
	nfmw->addChild(navQuitButton);

	auto navEditButton = make_sptr<Button>();
	navEditButton->attributes().padding.setValue(Padding { 4.F, 2.F });
	//navEditButton->attr.color.setValue(color::Dark::darkRed);

	auto navEditText = make_sptr<Text>();
	navEditText->attr.font.setValue(*defaultFont);
	navEditText->attr.fontSize.setValue(16.F);
	navEditText->attr.color.setValue(color::Dark::grey);
	navEditButton->setChild(navEditText);
	navEditText->setText("Edit");

	auto navWindowButton = make_sptr<Button>();
	navWindowButton->attributes().padding.setValue(Padding { 4.F, 2.F });
	//navWindowButton->attr.color.setValue(color::Dark::darkRed);

	auto navWindowText = make_sptr<Text>();
	navWindowText->attr.font.setValue(*defaultFont);
	navWindowText->attr.fontSize.setValue(16.F);
	navWindowText->attr.color.setValue(color::Dark::grey);

	navWindowButton->setChild(navWindowText);
	navWindowText->setText("Window");

	auto navProjectButton = make_sptr<Button>();
	navProjectButton->attributes().padding.setValue(Padding { 4.F, 2.F });
	//navProjectButton->attr.color.setValue(color::Dark::darkRed);

	auto navProjectText = make_sptr<Text>();
	navProjectText->attr.font.setValue(*defaultFont);
	navProjectText->attr.fontSize.setValue(16.F);
	navProjectText->attr.color.setValue(color::Dark::grey);

	navProjectButton->setChild(navProjectText);
	navProjectText->setText("Project");

	auto navMoreButton = make_sptr<Button>();
	navMoreButton->attributes().padding.setValue(Padding { 4.F, 2.F });
	//navMoreButton->attr.color.setValue(color::Dark::darkRed);

	auto navMoreText = make_sptr<Text>();
	navMoreText->attr.font.setValue(*defaultFont);
	navMoreText->attr.fontSize.setValue(16.F);
	navMoreText->attr.color.setValue(color::Dark::grey);

	navMoreButton->setChild(navMoreText);
	navMoreText->setText("More");

	auto navHelpButton = make_sptr<Button>();
	navHelpButton->attributes().padding.setValue(Padding { 4.F, 2.F });
	//navHelpButton->attr.color.setValue(color::Dark::darkRed);

	auto navHelpText = make_sptr<Text>();
	navHelpText->attr.font.setValue(*defaultFont);
	navHelpText->attr.fontSize.setValue(16.F);
	navHelpText->attr.color.setValue(color::Dark::grey);

	navHelpButton->setChild(navHelpText);
	navHelpText->setText("Help");

	//
	navSection->addChild(navBrandIcon);
	navSection->addChild(navFileButton);
	navSection->addChild(navEditButton);
	navSection->addChild(navWindowButton);
	navSection->addChild(navProjectButton);
	navSection->addChild(navMoreButton);
	navSection->addChild(navHelpButton);

	/**/

	auto leftSection = make_sptr<VerticalPanel>();
	leftSection->attr.width.setValue({ ReflowUnitType::eRelative, .2F });
	leftSection->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	leftSection->attr.height.setValue({ ReflowUnitType::eRelative, 1.F });
	leftSection->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
	leftSection->attr.justify.setValue(ReflowSpacing::eSpaceBetween);
	leftSection->attr.flexGrow.setValue(1.F);
	leftSection->attr.flexShrink.setValue(1.F);

	auto mainSection = make_sptr<VerticalPanel>();
	mainSection->attr.width.setValue({ ReflowUnitType::eRelative, .8F });
	mainSection->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	mainSection->attr.height.setValue({ ReflowUnitType::eRelative, 1.F });
	mainSection->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
	mainSection->attr.justify.setValue(ReflowSpacing::eStart);
	mainSection->attr.flexGrow.setValue(1.F);
	mainSection->attr.flexShrink.setValue(1.F);

	contentSection->addChild(leftSection);
	contentSection->addChild(mainSection);

	/**/

	#if false
    auto rightSection = make_sptr<ReflowContainer>();
    contentSection->add(rightSection);
	#endif

	#pragma region Left Section

	/**
	 * Section (Left)
	 */
	const auto shp = editorUi_.getSceneHierarchy()->makePanel();
	leftSection->addChild(shp);

	shp->attr.minHeight.setValue({ ReflowUnitType::eRelative, 1.F / 3.F });
	shp->attr.height.setValue({ ReflowUnitType::eRelative, 1.F / 3.F });
	shp->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F / 3.F });

	/**/

	const auto oep = editorUi_.getObjectEditor()->makePanel();
	leftSection->addChild(oep);

	oep->attr.minHeight.setValue({ ReflowUnitType::eRelative, 2.F / 3.F });
	oep->attr.height.setValue({ ReflowUnitType::eRelative, 2.F / 3.F });
	oep->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 2.F / 3.F });

	#pragma endregion
	#pragma region Main Section

	/**
	 * Section (Main)
	 */

	auto mainTopSection = make_sptr<VerticalPanel>();
	mainTopSection->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	mainTopSection->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	mainTopSection->attr.height.setValue({ ReflowUnitType::eRelative, 2.F / 3.F });
	mainTopSection->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 2.F / 3.F });
	mainTopSection->attr.flexGrow.setValue(1.F);
	mainTopSection->attr.flexShrink.setValue(1.F);

	auto assetBrowsePanel = editorUi_.getAssetBrowser()->makePanel();
	assetBrowsePanel->attr.height.setValue({ ReflowUnitType::eRelative, 1.F / 3.F });
	assetBrowsePanel->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F / 3.F });

	mainSection->addChild(mainTopSection);
	mainSection->addChild(assetBrowsePanel);

	if (true) {
		configureMainViewport(editorUi_, mainTopSection);
	} else {
		configureMainGraph(mainTopSection);
	}

	#pragma endregion

	/**/

	window_->setContent(root);

	/**/

	{
		auto* session = static_cast<ptr<engine::core::Session>>(GetSession().unwrap().get());
		auto& registry = session->getState().getRegistry();

		Vector<ptr<Actor>> actors {};

		auto* const pool = registry.getOrCreateActorPool<Actor>();
		const auto& storage = pool->__get_storage();

		// Error: Dual-Layer Iterators are broken
		const auto end = storage.cend();
		for (auto iter = storage.cbegin(); iter != end; ++iter) {
			if (iter.get_key() != invalid_actor_guid) {
				const auto& actor = iter.get_value();
				actors.push_back(std::addressof(const_cast<ref<Actor>>(actor)));
			}
		}

		if (not actors.empty()) {
			::storeEditorSelectedTarget(editorUi_, actors.front());
			storeHierarchyActor(editorUi_, actors);
		}
	}

}

#pragma endregion Temporary

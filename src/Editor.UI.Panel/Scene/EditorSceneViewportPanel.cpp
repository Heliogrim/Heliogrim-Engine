#include "EditorSceneViewportPanel.hpp"

#include <Editor.Core/EditorEngine.hpp>
#include <Editor.Core/HeliogrimEditor.hpp>
#include <Editor.UI/Widget/Viewport/EditorSceneViewport.hpp>
#include <Engine.ACS/ActorModule.hpp>
#include <Engine.Common/Discard.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.GFX.Scene/RenderSceneManager.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX/RenderTarget.hpp>
#include <Engine.GFX/Swapchain/VkSwapchain.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Reflow.Uikit/Atom/Text.hpp>
#include <Engine.Reflow.Uikit/Exp/Button.hpp>
#include <Engine.Reflow.Uikit/Exp/Card.hpp>
#include <Engine.Reflow/Widget/Viewport.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/HorizontalLayout.hpp>
#include <Heliogrim/Actor/Camera/CameraActor.hpp>

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

EditorSceneViewportPanel::EditorSceneViewportPanel() :
	VerticalLayout() {}

static void configureCtrls(cref<sptr<uikit::HorizontalLayout>> parent_) {

	auto viewText = make_sptr<uikit::Text>();
	viewText->setText("View");
	auto viewButton = uikit::makeButton(uikit::TextButtonCreateOptions { .level = 1, .text = ::hg::move(viewText) });
	::hg::discard = viewButton->onClick(
		[]([[maybe_unused]] const auto&) {
			IM_CORE_INFO("View Button Clicked.");
			return EventResponse::eConsumed;
		}
	);

	parent_->addChild(::hg::move(viewButton));

	/**/

	auto selectText = make_sptr<uikit::Text>();
	selectText->setText("Select");
	auto selectButton = uikit::makeButton(uikit::TextButtonCreateOptions { .level = 1, .text = ::hg::move(selectText) });
	::hg::discard = selectButton->onClick(
		[]([[maybe_unused]] const auto&) {
			IM_CORE_INFO("Select Button Clicked.");
			return EventResponse::eConsumed;
		}
	);

	parent_->addChild(::hg::move(selectButton));

	/**/

	auto addText = make_sptr<uikit::Text>();
	addText->setText("Add");
	auto addButton = uikit::makeButton(uikit::TextButtonCreateOptions { .level = 1, .text = ::hg::move(addText) });
	::hg::discard = addButton->onClick(
		[]([[maybe_unused]] const auto&) {
			IM_CORE_INFO("Add Button Clicked.");
			return EventResponse::eConsumed;
		}
	);

	parent_->addChild(::hg::move(addButton));

	/**/

	auto objectText = make_sptr<uikit::Text>();
	objectText->setText("Object");
	auto objectButton = uikit::makeButton(uikit::TextButtonCreateOptions { .level = 1, .text = ::hg::move(objectText) });
	::hg::discard = objectButton->onClick(
		[]([[maybe_unused]] const auto&) {
			IM_CORE_INFO("Object Button Clicked.");
			return EventResponse::eConsumed;
		}
	);

	parent_->addChild(::hg::move(objectButton));
}

static void configureViewport(cref<sptr<uikit::HorizontalLayout>> parent_) {
	auto viewport = make_sptr<EditorSceneViewport>();
	// TODO: Handle Viewport UI Scaling
	//viewport->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	//viewport->attr.height.setValue({ ReflowUnitType::eRelative, 1.F });
	parent_->addChild(viewport);

	/**/

	editor::EditorEngine::getEngine()->getActors()->getRegistry()->getOrCreateActorPool<CameraActor>();

	auto session = HeliogrimEditor::getSession();
	auto universe = GetUniverse(session);
	auto camera = CreateActor<CameraActor>();

	/**/

	camera->getRootComponent()->getLocalTransform().setLocation(math::Location { 0.F, 0.F, -5.F });
	auto trackedActor = universe.addActor(std::move(camera));

	/**/

	viewport->setViewportTarget("Test-Renderer"sv, universe, trackedActor);
	viewport->rebuildView();

	/**/

	viewport->addResizeListener(
		[instance = viewport.get()](mref<smr<engine::gfx::Swapchain>> prev_, mref<smr<engine::gfx::Swapchain>> next_) {

			if (not instance->hasMountedRenderTarget()) {
				return;
			}

			auto* const manager = engine::Engine::getEngine()->getGraphics()->getSceneManager();
			manager->transitionToTarget(::hg::move(prev_), ::hg::move(next_));
		}
	);

	/**/

	//auto viewportOverlay = make_sptr<Overlay>();
	auto viewOverBox = make_sptr<uikit::HorizontalLayout>();

	//parent_->addChild(viewportOverlay);
	/*viewportOverlay->setContent(viewOverBox);*/ {
		auto statsWrapper = make_sptr<uikit::VerticalLayout>();

		viewOverBox->addChild(statsWrapper);

		/**/

		auto sceneName = make_sptr<uikit::Text>();
		sceneName->getStyleAttributes().update<attr::TextStyle::color>(engine::color { 229.F, 190.F, 1.F, 255.F });
		sceneName->setText("Test Scene");

		statsWrapper->addChild(sceneName);

		auto sceneCalcTime = make_sptr<uikit::Text>();
		sceneCalcTime->getStyleAttributes().update<attr::TextStyle::color>(engine::color { 229.F, 190.F, 1.F, 255.F });
		sceneCalcTime->setText("<$ftime> ms");

		statsWrapper->addChild(sceneCalcTime);

		auto sceneFrames = make_sptr<uikit::Text>();
		sceneFrames->getStyleAttributes().update<attr::TextStyle::color>(engine::color { 229.F, 190.F, 1.F, 255.F });
		sceneFrames->setText("<$frames> FPS");

		statsWrapper->addChild(sceneFrames);
	}
}

sptr<EditorSceneViewportPanel> EditorSceneViewportPanel::make() {

	auto panel { std::shared_ptr<EditorSceneViewportPanel>(new EditorSceneViewportPanel()) };

	auto ctrls = make_sptr<uikit::HorizontalLayout>();

	auto wrapper = make_sptr<uikit::HorizontalLayout>();
	std::get<1>(wrapper->getLayoutAttributes().attributeSets).update<attr::FlexLayout::justify>(ReflowSpacing::eSpaceAround);

	/**/

	configureCtrls(ctrls);
	configureViewport(wrapper);

	/**/

	auto ctrlCard = uikit::makeCard({ .level = 1, .children = ::hg::move(ctrls) });
	ctrlCard->getLayoutAttributes().update<attr::BoxLayout::minHeight>({ ReflowUnitType::eAbsolute, 28.F });
	ctrlCard->getLayoutAttributes().update<attr::BoxLayout::maxHeight>({ ReflowUnitType::eAbsolute, 28.F });

	/**/

	panel->addChild(::hg::move(ctrlCard));
	panel->addChild(wrapper);

	return panel;
}

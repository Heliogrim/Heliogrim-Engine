#include "SceneEditorView.hpp"

#include "SceneEditorController.hpp"

import Editor.UI.Main;
import Editor.UI.Panel;
import Engine.Reflow;
import Engine.Reflow.Uikit;

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

[[nodiscard]] static sptr<FocusScope> makePanelScope(sptr<Widget> card_) {
	auto scope = make_sptr<FocusScope>(FocusScopeKind::ePanel);
	scope->setContent(::hg::move(card_));
	return scope;
}

UniquePtr<SceneEditorView> editor::ui::makeSceneEditorView(
	ref<const SceneEditorController> controller_
) {
	auto shell = make_sptr<uikit::HorizontalLayout>(); {
		auto& layout = shell->getLayoutAttributes();
		auto& box = std::get<0>(layout.attributeSets);
		auto& flex = std::get<1>(layout.attributeSets);

		box.update<attr::BoxLayout::maxWidth>(ReflowUnit { ReflowUnitType::eRelative, 1.F });
		box.update<attr::BoxLayout::maxHeight>(ReflowUnit { ReflowUnitType::eRelative, 1.F });

		box.update<attr::BoxLayout::widthGrow>(1.F);
		box.update<attr::BoxLayout::heightGrow>(1.F);

		flex.update<attr::FlexLayout::justify>(ReflowSpacing::eSpaceEvenly);
		flex.update<attr::FlexLayout::align>(ReflowAlignment::eStart);
		box.update<attr::BoxLayout::padding>({ 42.f, 12.f, 42.f, 44.f });

		flex.update<attr::FlexLayout::colGap>(8.F);
	}

	/**/

	auto mainRest = make_sptr<uikit::VerticalLayout>();

	[](ref<SharedPtr<uikit::VerticalLayout>> mainRest_) {
		auto& layout = mainRest_->getLayoutAttributes();
		auto& box = std::get<0>(layout.attributeSets);
		auto& flex = std::get<1>(layout.attributeSets);

		/**/

		box.update<attr::BoxLayout::maxWidth>(ReflowUnit { ReflowUnitType::eRelative, 1.F });
		box.update<attr::BoxLayout::minHeight>(ReflowUnit { ReflowUnitType::eRelative, 1.F });
		box.update<attr::BoxLayout::maxHeight>(ReflowUnit { ReflowUnitType::eRelative, 1.F });
		box.update<attr::BoxLayout::widthGrow>(1.F);
		box.update<attr::BoxLayout::heightGrow>(1.F);

		flex.update<attr::FlexLayout::justify>(ReflowSpacing::eStart);
		flex.update<attr::FlexLayout::align>(ReflowAlignment::eStart);

		flex.update<attr::FlexLayout::rowGap>(8.F);
	}(mainRest);

	[](ref<const SceneEditorController> controller_, ref<SharedPtr<uikit::VerticalLayout>> mainRest_) {
		auto top = make_sptr<uikit::HorizontalLayout>();

		std::get<0>(top->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>(
			ReflowUnit { ReflowUnitType::eRelative, 1.F }
		);
		std::get<0>(top->getLayoutAttributes().attributeSets).update<attr::BoxLayout::heightGrow>(1.F);
		std::get<0>(top->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);

		std::get<1>(top->getLayoutAttributes().attributeSets).update<attr::FlexLayout::justify>(ReflowSpacing::eStart);
		std::get<1>(top->getLayoutAttributes().attributeSets).update<attr::FlexLayout::align>(ReflowAlignment::eStart);

		std::get<1>(top->getLayoutAttributes().attributeSets).update<attr::FlexLayout::rowGap>(8.F);

		/**/

		auto sceneViewCard = controller_.sceneHierarchy->getView().main;
		auto sceneViewBox = make_sptr<uikit::BoxLayout>();

		sceneViewCard->getLayoutAttributes().update<attr::BoxLayout::heightGrow>(1.F);
		sceneViewCard->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);

		sceneViewBox->getLayoutAttributes().update<attr::BoxLayout::heightGrow>(1.F);
		sceneViewBox->getLayoutAttributes().update<attr::BoxLayout::minWidth>({ ReflowUnitType::eAbsolute, 320.F });
		sceneViewBox->getLayoutAttributes().update<attr::BoxLayout::maxWidth>({ ReflowUnitType::eRelative, 1.F });
		sceneViewBox->getLayoutAttributes().update<attr::BoxLayout::minHeight>(ReflowUnit { ReflowUnitType::eRelative, 1.F });
		sceneViewBox->getLayoutAttributes().update<attr::BoxLayout::maxHeight>(ReflowUnit { ReflowUnitType::eRelative, 1.F });

		sceneViewBox->setChild(makePanelScope(::hg::move(sceneViewCard)));
		top->addChild(::hg::move(sceneViewBox));

		/**/

		auto esvp = EditorSceneViewportPanel::make();

		std::get<0>(esvp->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>(
			ReflowUnit { ReflowUnitType::eRelative, 1.F }
		);
		std::get<0>(esvp->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
		std::get<0>(esvp->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>(
			ReflowUnit { ReflowUnitType::eRelative, 1.F }
		);
		std::get<0>(esvp->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>(
			ReflowUnit { ReflowUnitType::eRelative, 1.F }
		);

		std::get<1>(esvp->getLayoutAttributes().attributeSets).update<attr::FlexLayout::rowGap>(8.F);

		top->addChild(makePanelScope(::hg::move(esvp)));
		mainRest_->addChild(::hg::move(top));
	}(controller_, mainRest);

	[](ref<const SceneEditorController> controller_, ref<SharedPtr<uikit::VerticalLayout>> mainRest_) {

		auto assetBrowserCard = controller_.assetBrowser->getView().main;

		assetBrowserCard->getLayoutAttributes().update<attr::BoxLayout::minWidth>({ ReflowUnitType::eRelative, 1.F });
		assetBrowserCard->getLayoutAttributes().update<attr::BoxLayout::maxWidth>({ ReflowUnitType::eRelative, 1.F });
		assetBrowserCard->getLayoutAttributes().update<attr::BoxLayout::minHeight>(ReflowUnit { ReflowUnitType::eAbsolute, 320.F });
		assetBrowserCard->getLayoutAttributes().update<attr::BoxLayout::maxHeight>(ReflowUnit { ReflowUnitType::eAbsolute, 320.F });

		mainRest_->addChild(makePanelScope(::hg::move(assetBrowserCard)));
	}(controller_, mainRest);

	/**/

	auto right = make_sptr<uikit::VerticalLayout>();

	[](ref<SharedPtr<uikit::VerticalLayout>> right_) static {
		auto& layout = right_->getLayoutAttributes();
		auto& box = std::get<0>(layout.attributeSets);
		auto& flex = std::get<1>(layout.attributeSets);

		/**/

		box.update<attr::BoxLayout::minWidth>(ReflowUnit { ReflowUnitType::eAbsolute, 360.F });
		box.update<attr::BoxLayout::maxWidth>(ReflowUnit { ReflowUnitType::eAbsolute, 360.F });
		box.update<attr::BoxLayout::heightGrow>(1.F);

		flex.update<attr::FlexLayout::justify>(ReflowSpacing::eStart);
		flex.update<attr::FlexLayout::align>(ReflowAlignment::eStart);

		flex.update<attr::FlexLayout::rowGap>(8.F);
	}(right);

	[](ref<const SceneEditorController> controller_, ref<SharedPtr<uikit::VerticalLayout>> right_) static {
		auto propertiesCard = controller_.properties->getView().main;
		auto propertiesBox = make_sptr<uikit::BoxLayout>();

		propertiesCard->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
		propertiesCard->getLayoutAttributes().update<attr::BoxLayout::heightGrow>(1.F);

		propertiesBox->getLayoutAttributes().update<attr::BoxLayout::minWidth>({ ReflowUnitType::eRelative, 1.F });
		propertiesBox->getLayoutAttributes().update<attr::BoxLayout::maxWidth>({ ReflowUnitType::eRelative, 1.F });
		propertiesBox->getLayoutAttributes().update<attr::BoxLayout::heightGrow>(1.F);

		propertiesBox->setChild(makePanelScope(::hg::move(propertiesCard)));
		right_->addChild(::hg::move(propertiesBox));
	}(controller_, right);

	/**/

	shell->addChild(::hg::move(mainRest));
	shell->addChild(::hg::move(right));
	return make_uptr<SceneEditorView>(::hg::move(shell));
}

#include "SceneEditorView.hpp"

#include <Editor.UI.Main/EditorUI.hpp>
#include <Editor.UI.Main/Module/EditorUI.hpp>
#include <Editor.UI.Panel/Assets/AssetBrowserController.hpp>
#include <Editor.UI.Panel/Assets/AssetBrowserView.hpp>
#include <Editor.UI.Panel/Scene/EditorSceneViewportPanel.hpp>
#include <Editor.UI.Panel/Scene/Hierarchy/SceneHierarchyController.hpp>
#include <Editor.UI.Panel/Scene/Hierarchy/SceneHierarchyView.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/BoxLayout.hpp>

#include "SceneEditorController.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

UniquePtr<SceneEditorView> editor::ui::makeSceneEditorView(
	ref<const SceneEditorController> controller_
) {
	auto main = make_sptr<uikit::HorizontalLayout>(); {
		auto& layout = main->getLayoutAttributes();
		auto& box = std::get<0>(layout.attributeSets);
		auto& flex = std::get<1>(layout.attributeSets);

		box.update<attr::BoxLayout::minWidth>({ ReflowUnitType::eRelative, 1.F });
		box.update<attr::BoxLayout::minHeight>({ ReflowUnitType::eAuto, 0.F });

		flex.update<attr::FlexLayout::justify>(ReflowSpacing::eSpaceEvenly);
		flex.update<attr::FlexLayout::align>(ReflowAlignment::eStart);

		flex.update<attr::FlexLayout::colGap>(8.F);
	}

	/**/

	auto left = make_sptr<uikit::VerticalLayout>(); {
		auto& layout = left->getLayoutAttributes();
		auto& box = std::get<0>(layout.attributeSets);
		auto& flex = std::get<1>(layout.attributeSets);

		/**/

		box.update<attr::BoxLayout::minWidth>(ReflowUnit { ReflowUnitType::eAbsolute, 360.F });
		box.update<attr::BoxLayout::maxWidth>(ReflowUnit { ReflowUnitType::eAbsolute, 360.F });
		box.update<attr::BoxLayout::minHeight>(ReflowUnit { ReflowUnitType::eRelative, 1.F });
		box.update<attr::BoxLayout::maxHeight>(ReflowUnit { ReflowUnitType::eRelative, 1.F });

		flex.update<attr::FlexLayout::justify>(ReflowSpacing::eStart);
		flex.update<attr::FlexLayout::align>(ReflowAlignment::eStart);

		flex.update<attr::FlexLayout::rowGap>(8.F);
	} {
		auto sceneViewCard = controller_.sceneHierarchy->getView().main;
		auto sceneViewBox = make_sptr<uikit::BoxLayout>();

		sceneViewCard->getLayoutAttributes().update<attr::BoxLayout::minWidth>({ ReflowUnitType::eRelative, 1.F });
		sceneViewCard->getLayoutAttributes().update<attr::BoxLayout::maxWidth>({ ReflowUnitType::eRelative, 1.F });
		sceneViewCard->getLayoutAttributes().update<attr::BoxLayout::minHeight>({ ReflowUnitType::eRelative, 1.F });
		sceneViewCard->getLayoutAttributes().update<attr::BoxLayout::maxHeight>({ ReflowUnitType::eRelative, 1.F });

		sceneViewBox->getLayoutAttributes().update<attr::BoxLayout::minWidth>({ ReflowUnitType::eRelative, 1.F });
		sceneViewBox->getLayoutAttributes().update<attr::BoxLayout::maxWidth>({ ReflowUnitType::eRelative, 1.F });
		sceneViewBox->getLayoutAttributes().update<attr::BoxLayout::minHeight>(
			ReflowUnit { ReflowUnitType::eRelative, 2.F / 3.F }
		);
		sceneViewBox->getLayoutAttributes().update<attr::BoxLayout::maxHeight>(
			ReflowUnit { ReflowUnitType::eRelative, 2.F / 3.F }
		);

		sceneViewBox->setChild(sceneViewCard);
		left->addChild(::hg::move(sceneViewBox));

		/**/

		auto sceneStatsBox = make_sptr<uikit::BoxLayout>();

		sceneStatsBox->getLayoutAttributes().update<attr::BoxLayout::minWidth>({ ReflowUnitType::eRelative, 1.F });
		sceneStatsBox->getLayoutAttributes().update<attr::BoxLayout::maxWidth>({ ReflowUnitType::eRelative, 1.F });
		sceneStatsBox->getLayoutAttributes().update<attr::BoxLayout::minHeight>(
			{ ReflowUnitType::eRelative, 1.F / 3.F }
		);
		sceneStatsBox->getLayoutAttributes().update<attr::BoxLayout::maxHeight>(
			{ ReflowUnitType::eRelative, 1.F / 3.F }
		);

		left->addChild(::hg::move(sceneStatsBox));
	}

	/**/

	auto center = make_sptr<uikit::VerticalLayout>(); {
		auto& layout = center->getLayoutAttributes();
		auto& box = std::get<0>(layout.attributeSets);
		auto& flex = std::get<1>(layout.attributeSets);

		/**/

		box.update<attr::BoxLayout::minWidth>(ReflowUnit { ReflowUnitType::eRelative, 1.F });
		box.update<attr::BoxLayout::maxWidth>(ReflowUnit { ReflowUnitType::eRelative, 1.F });
		box.update<attr::BoxLayout::minHeight>(ReflowUnit { ReflowUnitType::eRelative, 1.F });
		box.update<attr::BoxLayout::maxHeight>(ReflowUnit { ReflowUnitType::eRelative, 1.F });

		flex.update<attr::FlexLayout::justify>(ReflowSpacing::eStart);
		flex.update<attr::FlexLayout::align>(ReflowAlignment::eCenter);

		flex.update<attr::FlexLayout::rowGap>(8.F);
	} {
		auto esvp = EditorSceneViewportPanel::make();

		std::get<0>(esvp->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minWidth>(
			ReflowUnit { ReflowUnitType::eRelative, 1.F }
		);
		std::get<0>(esvp->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>(
			ReflowUnit { ReflowUnitType::eRelative, 1.F }
		);
		std::get<0>(esvp->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>(
			ReflowUnit { ReflowUnitType::eRelative, 2.F / 3.F }
		);
		std::get<0>(esvp->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>(
			ReflowUnit { ReflowUnitType::eRelative, 2.F / 3.F }
		);

		std::get<1>(esvp->getLayoutAttributes().attributeSets).update<attr::FlexLayout::rowGap>(8.F);

		center->addChild(esvp);

		/**/

		auto assetBrowserCard = controller_.assetBrowser->getView().main;

		assetBrowserCard->getLayoutAttributes().update<attr::BoxLayout::minWidth>({ ReflowUnitType::eRelative, 1.F });
		assetBrowserCard->getLayoutAttributes().update<attr::BoxLayout::maxWidth>({ ReflowUnitType::eRelative, 1.F });
		assetBrowserCard->getLayoutAttributes().update<attr::BoxLayout::minHeight>(
			ReflowUnit { ReflowUnitType::eRelative, 1.F / 3.F }
		);
		assetBrowserCard->getLayoutAttributes().update<attr::BoxLayout::maxHeight>(
			ReflowUnit { ReflowUnitType::eRelative, 1.F / 3.F }
		);

		center->addChild(::hg::move(assetBrowserCard));
	}

	/**/

	auto right = make_sptr<uikit::VerticalLayout>(); {
		auto& layout = right->getLayoutAttributes();
		auto& box = std::get<0>(layout.attributeSets);
		auto& flex = std::get<1>(layout.attributeSets);

		/**/

		box.update<attr::BoxLayout::minWidth>(ReflowUnit { ReflowUnitType::eAbsolute, 360.F });
		box.update<attr::BoxLayout::maxWidth>(ReflowUnit { ReflowUnitType::eAbsolute, 360.F });
		box.update<attr::BoxLayout::minHeight>(ReflowUnit { ReflowUnitType::eRelative, 1.F });
		box.update<attr::BoxLayout::maxHeight>(ReflowUnit { ReflowUnitType::eRelative, 1.F });

		flex.update<attr::FlexLayout::justify>(ReflowSpacing::eStart);
		flex.update<attr::FlexLayout::align>(ReflowAlignment::eStart);

		flex.update<attr::FlexLayout::rowGap>(8.F);
	} {
		auto* editorUi = static_cast<ptr<editor::EditorUI>>(
			engine::Engine::getEngine()->getModules().getSubModule(editor::EditorUIDepKey).get()
		);

		/**/

		auto objectEditorBox = make_sptr<uikit::BoxLayout>();

		objectEditorBox->getLayoutAttributes().update<attr::BoxLayout::minWidth>({ ReflowUnitType::eRelative, 1.F });
		objectEditorBox->getLayoutAttributes().update<attr::BoxLayout::maxWidth>({ ReflowUnitType::eRelative, 1.F });
		objectEditorBox->getLayoutAttributes().update<attr::BoxLayout::minHeight>(ReflowUnit { ReflowUnitType::eRelative, 1.F });
		objectEditorBox->getLayoutAttributes().update<attr::BoxLayout::maxHeight>(ReflowUnit { ReflowUnitType::eRelative, 1.F });

		right->addChild(::hg::move(objectEditorBox));
	}

	/**/

	main->addChild(::hg::move(left));
	main->addChild(::hg::move(center));
	main->addChild(::hg::move(right));
	return make_uptr<SceneEditorView>(::hg::move(main));
}

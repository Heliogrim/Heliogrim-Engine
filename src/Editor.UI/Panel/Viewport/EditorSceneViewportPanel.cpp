#include "EditorSceneViewportPanel.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Widget/Button.hpp>
#include <Engine.Reflow/Widget/HorizontalPanel.hpp>
#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Reflow/Widget/Viewport.hpp>

#include "../../Color/Dark.hpp"
#include "../../Theme/Theme.hpp"
#include "../../Widget/Viewport/EditorSceneViewport.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

EditorSceneViewportPanel::EditorSceneViewportPanel() :
	VerticalPanel() {}

static void configureCtrls(cref<sptr<HorizontalPanel>> parent_) {

	const auto theme = Theme::get();

	auto playButton = make_sptr<Button>();
	theme->applyTextButton(playButton);

	auto pauseButton = make_sptr<Button>();
	theme->applyTextButton(pauseButton);

	auto stopButton = make_sptr<Button>();
	theme->applyTextButton(stopButton);

	/**/

	auto playText = make_sptr<Text>();
	theme->applyText(playText);

	auto pauseText = make_sptr<Text>();
	theme->applyText(pauseText);

	auto stopText = make_sptr<Text>();
	theme->applyText(stopText);

	/**/

	playText->setText("Play");
	playButton->setChild(playText);
	parent_->addChild(playButton);

	/**/

	pauseText->setText("Pause");
	pauseButton->setChild(pauseText);
	parent_->addChild(pauseButton);

	/**/

	stopText->setText("Play");
	stopButton->setChild(stopText);
	parent_->addChild(stopButton);
}

static void configureViewport(cref<sptr<HorizontalPanel>> parent_) {
	auto viewport = make_sptr<EditorSceneViewport>();
	viewport->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	viewport->attr.height.setValue({ ReflowUnitType::eRelative, 1.F });
	parent_->addChild(viewport);
}

sptr<EditorSceneViewportPanel> EditorSceneViewportPanel::make() {

	auto panel { std::shared_ptr<EditorSceneViewportPanel>(new EditorSceneViewportPanel()) };

	auto ctrls = make_sptr<HorizontalPanel>();
	ctrls->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	ctrls->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 24.F });
	ctrls->attr.justify.setValue(ReflowSpacing::eSpaceAround);

	auto wrapper = make_sptr<HorizontalPanel>();
	wrapper->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	wrapper->attr.height.setValue({ ReflowUnitType::eRelative, 1.F });
	wrapper->attr.justify.setValue(ReflowSpacing::eSpaceAround);
	wrapper->attr.flexShrink.setValue(1.F);

	configureCtrls(ctrls);
	configureViewport(wrapper);

	panel->addChild(ctrls);
	panel->addChild(wrapper);

	return panel;
}

#include "ProjectViewportPanel.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Widget/Button.hpp>
#include <Engine.Reflow/Widget/Text.hpp>
#include "Engine.Reflow/Widget/Viewport.hpp"

#include "../Style/Style.hpp"
#include <Engine.Reflow/Style/BoundStyleSheet.hpp>

#include "Editor.UI/Color/Dark.hpp"
#include "Engine.GFX.Glow.UI/TestUI.hpp"

using namespace ember::editor::ui;
using namespace ember::engine::reflow;
using namespace ember;

ProjectViewportPanel::ProjectViewportPanel() :
    Panel(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::DefaultKey))) {}

static void configureCtrls(cref<sptr<HBox>> parent_) {

    auto playButton = make_sptr<Button>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::TextButtonKey)));
    auto pauseButton = make_sptr<Button>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::TextButtonKey)));
    auto stopButton = make_sptr<Button>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::TextButtonKey)));

    /**/

    auto* font { getDefaultFont() };
    const StyleSheet titleStyle {
        .color = { true, color::Dark::white },
        .font = { true, font },
        .fontSize = { true, 16.F },
    };

    auto playText = make_sptr<Text>(BoundStyleSheet::make(titleStyle));
    auto pauseText = make_sptr<Text>(BoundStyleSheet::make(titleStyle));
    auto stopText = make_sptr<Text>(BoundStyleSheet::make(titleStyle));

    /**/

    playText->setText("Play");
    playButton->addChild(playText);
    parent_->addChild(playButton);

    /**/

    pauseText->setText("Pause");
    pauseButton->addChild(pauseText);
    parent_->addChild(pauseButton);

    /**/

    stopText->setText("Play");
    stopButton->addChild(stopText);
    parent_->addChild(stopButton);
}

static void configureViewport(cref<sptr<HBox>> parent_) {
    auto viewport = make_sptr<Viewport>(BoundStyleSheet::make(StyleSheet {
        .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .height = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .color = { true, color::Dark::white }
    }));
    parent_->addChild(viewport);
}

sptr<Panel> ProjectViewportPanel::make() {

    auto panel { _STD shared_ptr<ProjectViewportPanel>(new ProjectViewportPanel()) };

    auto ctrls = make_sptr<HBox>(BoundStyleSheet::make(StyleSheet {
        .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .minHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 24.F } },
        .height = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
        .reflowSpacing = { true, ReflowSpacing::eSpaceAround },
        .color = { true, color::Dark::backgroundDefault },
    }));
    auto wrapper = make_sptr<HBox>(BoundStyleSheet::make(StyleSheet {
        .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .height = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .reflowSpacing = { true, ReflowSpacing::eSpaceAround },
        .reflowShrink = { true, 1.F },
        .color = { true, color::Dark::backgroundDefault },
    }));

    configureCtrls(ctrls);
    configureViewport(wrapper);

    panel->addChild(ctrls);
    panel->addChild(wrapper);

    return panel;
}

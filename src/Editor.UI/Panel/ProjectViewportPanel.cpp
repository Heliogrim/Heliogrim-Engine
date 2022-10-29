#include "ProjectViewportPanel.hpp"

#include <Engine.GFX.Glow.UI/Widget/Button.hpp>
#include <Engine.GFX.Glow.UI/Widget/Text.hpp>

#include "Engine.GFX.Glow.UI/Widget/Viewport.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember::editor::ui;
using namespace ember;

ProjectViewportPanel::ProjectViewportPanel() :
    Panel() {}

void configureCtrls(cref<sptr<Widget>> parent_) {
    parent_->setReflowType(ReflowType::eFlexRow);
    parent_->setReflowSpacing(ReflowSpacing::eSpaceAround);

    parent_->_color = engine::color { 46.F, 50.F, 58.F, 255.F };

    parent_->_extent.x = 1.F;
    parent_->_extent.y = 0.F;
    parent_->_minExtent.x = -1.F;
    parent_->_minExtent.y = 24.F;

    /**/

    auto playButton = make_sptr<Button>();
    auto pauseButton = make_sptr<Button>();
    auto stopButton = make_sptr<Button>();

    /**/

    auto playText = make_sptr<Text>();
    auto pauseText = make_sptr<Text>();
    auto stopText = make_sptr<Text>();

    /**/

    playText->_text = "Play"sv;
    playButton->add(playText);

    playButton->_color = engine::color { 46.F, 50.F, 58.F, 255.F };
    playButton->_margin = math::vec4 { 4.F };
    playButton->_padding = math::vec4 { 8.F, 4.F };
    parent_->add(playButton);

    /**/

    pauseText->_text = "Pause"sv;
    pauseButton->add(pauseText);

    pauseButton->_color = engine::color { 46.F, 50.F, 58.F, 255.F };
    pauseButton->_margin = math::vec4 { 4.F };
    pauseButton->_padding = math::vec4 { 8.F, 4.F };
    parent_->add(pauseButton);

    /**/

    stopText->_text = "Play"sv;
    stopButton->add(stopText);

    stopButton->_color = engine::color { 46.F, 50.F, 58.F, 255.F };
    stopButton->_margin = math::vec4 { 4.F };
    stopButton->_padding = math::vec4 { 8.F, 4.F };
    parent_->add(stopButton);
}

void configureViewport(cref<sptr<Widget>> parent_) {
    parent_->setReflowType(ReflowType::eFlexRow);
    parent_->setReflowSpacing(ReflowSpacing::eSpaceAround);

    parent_->_color = engine::color { 46.F, 50.F, 58.F, 255.F };

    parent_->_extent.x = 1.F;
    parent_->_extent.y = 1.F;
    parent_->_reflowShrink = 1.F;

    /**/

    auto viewport = make_sptr<Viewport>();

    /**/

    viewport->_color = engine::color { 46.F, 50.F, 58.F, 255.F };
    viewport->_extent.x = 1.F;
    viewport->_extent.y = 1.F;

    /**/

    parent_->add(viewport);
}

sptr<Panel> ProjectViewportPanel::make() {

    auto panel { _STD shared_ptr<ProjectViewportPanel>(new ProjectViewportPanel()) };

    panel->setReflowType(ReflowType::eFlexCol);
    panel->setReflowSpacing(ReflowSpacing::eStart);

    auto ctrls = make_sptr<ReflowContainer>();
    auto wrapper = make_sptr<ReflowContainer>();

    configureCtrls(ctrls);
    configureViewport(wrapper);

    panel->add(ctrls);
    panel->add(wrapper);

    return panel;
}

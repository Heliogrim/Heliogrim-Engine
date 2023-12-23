#include "SceneHierarchyPanel.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Widget/HorizontalPanel.hpp>
#include <Engine.Reflow/Widget/Scroll/VScrollBox.hpp>
#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Reflow/Widget/Button.hpp>
#include <Engine.Reflow/Widget/Input/InputText.hpp>

#include "../Color/Dark.hpp"
#include "Editor.UI/Theme/Theme.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

SceneHierarchyPanel::SceneHierarchyPanel() :
    VerticalPanel(),
    _content(nullptr),
    _module(nullptr),
    _resolver(nullptr),
    _generator(nullptr) {
    /**/
    attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
    attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
    attr.height.setValue({ ReflowUnitType::eRelative, 1.F });
    attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
    attr.flexShrink.setValue(1.F);
    attr.flexGrow.setValue(1.F);
}

static void configureNav(cref<sptr<HorizontalPanel>> navBar_) {

    const auto* const theme = Theme::get();

    const auto title = make_sptr<Text>();
    theme->applyLabel(title);
    title->setText("Scene Hierarchy");
    navBar_->addChild(title);

    /**/

    const auto closeButton { make_sptr<Button>() };
    theme->applyTextButton(closeButton);

    const auto closeText { make_sptr<Text>() };
    theme->applyLabel(closeText);

    closeText->setText(R"(X)");
    closeButton->setChild(closeText);

    navBar_->addChild(closeButton);
}

void configureHeader(cref<sptr<VerticalPanel>> header_) {

    const auto* const theme = Theme::get();

    const auto row {
        make_sptr<HorizontalPanel>()
    };
    row->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
    row->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
    row->attr.height.setValue({ ReflowUnitType::eAbsolute, 20.F });
    row->attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });
    row->attr.flexShrink.setValue(1.F);
    row->attr.flexGrow.setValue(1.F);

    /**/

    const auto search = make_sptr<InputText>();

    search->_wrapper->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
    search->_wrapper->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
    search->_wrapper->attr.height.setValue({ ReflowUnitType::eAbsolute, 16.F });
    search->_wrapper->attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 16.F });
    search->_wrapper->attr.padding.setValue(Padding { 4.F, 2.F });
    search->_wrapper->attr.flexShrink.setValue(1.F);
    search->_wrapper->attr.flexGrow.setValue(1.F);

    theme->applyLabel(search->_text);
    search->setPlaceholder(R"(Search Hierarchy...)");

    row->addChild(search);

    /**/

    const auto homeBtn { make_sptr<Button>() };
    theme->applyTextButton(homeBtn);

    const auto homeTxt { make_sptr<Text>() };
    theme->applyLabel(homeTxt);

    homeTxt->setText(R"(H)");
    homeBtn->setChild(homeTxt);

    row->addChild(homeBtn);
}

sptr<SceneHierarchyPanel> SceneHierarchyPanel::make(const non_owning_rptr<SceneHierarchy> module_) {

    const auto* const theme = Theme::get();

    auto panel { _STD shared_ptr<SceneHierarchyPanel>(new SceneHierarchyPanel()) };
    panel->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 20.F + 20.F });

    /**/

    panel->_module = module_;

    /**/

    auto nav { make_sptr<HorizontalPanel>() };
    nav->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
    nav->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
    nav->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });
    nav->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
    nav->attr.padding.setValue(Padding { 4.F, 0.F });
    nav->attr.justify.setValue(ReflowSpacing::eSpaceBetween);

    panel->addChild(nav);
    configureNav(nav);

    /**/

    auto header { make_sptr<VerticalPanel>() };
    header->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
    header->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
    header->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });
    header->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
    header->attr.padding.setValue(Padding { 4.F, 0.F });
    header->attr.justify.setValue(ReflowSpacing::eSpaceBetween);

    panel->addChild(header);
    configureHeader(header);

    /**/

    auto content { make_sptr<VScrollBox>() };
    content->attr.height.setValue(ReflowUnitType::eRelative, 1.F);
    content->attr.width.setValue(ReflowUnitType::eRelative, 1.F);

    panel->_content = content;
    panel->addChild(content);

    return panel;
}

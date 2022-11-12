#include "SceneHierarchyPanel.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Style/BoundStyleSheet.hpp>
#include <Engine.Reflow/Widget/HBox.hpp>
#include <Engine.Reflow/Widget/Scroll/VScrollBox.hpp>
#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Reflow/Widget/Button.hpp>
#include <Engine.Reflow/Style/StyleCondition.hpp>
#include <Engine.Reflow/Widget/Input/InputText.hpp>

#include "../Style/Style.hpp"
#include "../Color/Dark.hpp"

using namespace ember::editor::ui;
using namespace ember::engine::reflow;
using namespace ember;

SceneHierarchyPanel::SceneHierarchyPanel() :
    Panel(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::AdoptFlexBoxKey))),
    _content(nullptr),
    _module(nullptr),
    _resolver(nullptr) {}

static void configureNav(cref<sptr<HBox>> navBar_) {

    const auto title { make_sptr<Text>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::TitleSmallKey))) };
    title->style().margin = Margin { 0.F, 4.F };
    title->setText("Scene Hierarchy");
    navBar_->addChild(title);

    /**/

    auto btnStyle { BoundStyleSheet::make() };
    btnStyle->pushStyle({ Style::ButtonKey, nullptr, Style::get()->getStyleSheet(Style::ButtonKey) });
    btnStyle->pushStyle({
        Style::ButtonRaisedKey,
        style::isRaised,
        Style::get()->getStyleSheet(Style::ButtonRaisedKey)
    });

    auto txtStyle { BoundStyleSheet::make() };
    txtStyle->pushStyle({ Style::TitleSmallKey, nullptr, Style::get()->getStyleSheet(Style::TitleSmallKey) });
    txtStyle->pushStyle({
        Style::ButtonRaisedKey,
        style::isNever,
        Style::get()->getStyleSheet(Style::TitleRaisedKey)
    });

    const auto closeButton { make_sptr<Button>(_STD move(btnStyle)) };
    const auto closeText { make_sptr<Text>(_STD move(txtStyle)) };

    closeText->setText(R"(X)");
    closeButton->addChild(closeText);

    navBar_->addChild(closeButton);
}

void configureHeader(cref<sptr<VBox>> header_) {

    const auto row { make_sptr<HBox>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::AdoptFlexBoxKey))) };
    row->style()->height = ReflowUnit { ReflowUnitType::eAbsolute, 20.F };
    row->style()->maxHeight = ReflowUnit { ReflowUnitType::eAbsolute, 20.F };

    /**/

    const auto search {
        make_sptr<InputText>(BoundStyleSheet::make(StyleSheet {
            .width { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .maxWidth { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .height { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
            .maxHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
            .padding { true, Padding { 4.F, 2.F } },
            .margin { true, Margin { 4.F, 2.F } },
            .borderRadius { true, BorderRadius { 4.F } },
            .color { true, color::Dark::backgroundInnerField }
        }), BoundStyleSheet::make(Style::get()->getStyleSheet(Style::TitleSmallKey)))
    };

    search->setPlaceholder(R"(Search Hierarchy...)");

    row->addChild(search);

    /**/

    auto btnStyle { BoundStyleSheet::make() };
    btnStyle->pushStyle({ Style::ButtonKey, nullptr, Style::get()->getStyleSheet(Style::ButtonKey) });
    btnStyle->pushStyle({
        Style::ButtonRaisedKey,
        style::isRaised,
        Style::get()->getStyleSheet(Style::ButtonRaisedKey)
    });

    auto txtStyle { BoundStyleSheet::make() };
    txtStyle->pushStyle({ Style::TitleSmallKey, nullptr, Style::get()->getStyleSheet(Style::TitleSmallKey) });
    txtStyle->pushStyle({
        Style::ButtonRaisedKey,
        style::isNever,
        Style::get()->getStyleSheet(Style::TitleRaisedKey)
    });

    const auto homeBtn { make_sptr<Button>(_STD move(btnStyle)) };
    const auto homeTxt { make_sptr<Text>(_STD move(txtStyle)) };

    homeTxt->setText(R"(H)");
    homeBtn->addChild(homeTxt);

    row->addChild(homeBtn);
}

sptr<SceneHierarchyPanel> SceneHierarchyPanel::make(const non_owning_rptr<SceneHierarchy> module_) {

    auto panel { _STD shared_ptr<SceneHierarchyPanel>(new SceneHierarchyPanel()) };
    panel->_style->minHeight = ReflowUnit { ReflowUnitType::eAbsolute, 20.F + 20.F };

    /**/

    panel->_module = module_;

    /**/

    auto nav { make_sptr<HBox>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::NavBarKey))) };
    panel->addChild(nav);

    configureNav(nav);

    /**/

    auto header { make_sptr<VBox>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::NavBarKey))) };
    header->style()->minHeight = ReflowUnit { ReflowUnitType::eAbsolute, 20.F };
    panel->addChild(header);

    configureHeader(header);

    /**/

    auto content { make_sptr<VScrollBox>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::ScrollBoxKey))) };
    content->style()->color = color::Dark::backgroundInnerField;

    panel->_content = content;
    panel->addChild(content);

    return panel;
}

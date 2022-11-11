#include "ObjectEditorPanel.hpp"

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

ObjectEditorPanel::ObjectEditorPanel() :
    Panel(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::AdoptFlexBoxKey))),
    _editor(nullptr),
    _mapper(nullptr) {}

ObjectEditorPanel::~ObjectEditorPanel() {
    clearEditor();
}

void ObjectEditorPanel::setEditorTarget(cref<type_id> typeId_, const ptr<void> obj_) {}

void ObjectEditorPanel::clearEditor() {

    if (_mapper == nullptr) {
        return;
    }

}

static void configureNav(cref<sptr<HBox>> navBar_) {

    const auto title { make_sptr<Text>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::TitleSmallKey))) };
    title->style().margin = Margin { 0.F, 4.F };
    title->setText("Object Editor");
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

sptr<ObjectEditorPanel> ObjectEditorPanel::make(const non_owning_rptr<ObjectEditor> editor_) {

    auto panel { sptr<ObjectEditorPanel>(new ObjectEditorPanel()) };
    panel->_style->minHeight = ReflowUnit { ReflowUnitType::eAbsolute, 20.F };

    /**/

    panel->_editor = editor_;

    /**/

    auto nav { make_sptr<HBox>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::NavBarKey))) };
    panel->addChild(nav);

    configureNav(nav);

    /**/

    return panel;
}

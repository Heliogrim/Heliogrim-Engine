#include "TreeItem.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "../../Style/BoundStyleSheet.hpp"

using namespace ember::engine::reflow;
using namespace ember;

[[nodiscard]] bool styleIsSelected(cref<sptr<Widget>> widget_) {
    return static_cast<ptr<TreeItem>>(widget_.get())->isSelected();
}

[[nodiscard]] sptr<BoundStyleSheet> makeStyleSheet() {

    auto style = BoundStyleSheet::make(
        StyleSheet {
            .minWidth { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .width { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .maxWidth { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .minHeight { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .height { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .maxHeight { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .wrap { true, ReflowWrap::eNoWrap },
            .padding { true, Padding { 0.F } },
            .margin { true, Margin { 0.F } },
            .color { true, engine::color { 0.F, 0.F, 0.F, 0.F } }
        }
    );

    style->pushStyle(
        {
            AssocKey<string>::from(R"(TreeItem::Selected)"),
            styleIsSelected,
            make_sptr<StyleSheet>()
        }
    );

    return style;
}

TreeItem::TreeItem() :
    HBox(makeStyleSheet()),
    _selected(false) {}

TreeItem::~TreeItem() = default;

bool TreeItem::isSelected() const noexcept {
    return _selected;
}

void TreeItem::setSelected(const bool selected_) {
    if (_selected != selected_) {
        this->markAsPending();
    }

    _selected = selected_;
}

sptr<void> TreeItem::getTreeDataItem() const noexcept {
    return _treeDataItem;
}

void TreeItem::setTreeDataItem(cref<sptr<void>> dataItem_) {
    _treeDataItem = dataItem_;
}

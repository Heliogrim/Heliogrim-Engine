#include "TreeView.hpp"

#include <format>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "TreeItem.hpp"

using namespace hg::engine::reflow;
using namespace hg;

static constexpr float ident_per_level = 12.F;
static constexpr float row_min_height = 20.F;

string TreeViewBase::getTag() const noexcept {
    return std::format(R"(TreeView <{:#x}>)", reinterpret_cast<u64>(this));
}

sptr<Widget> TreeViewBase::generateRow(cref<TreeViewItem> view_, cref<sptr<Widget>> content_) {

    auto wrapper = make_sptr<TreeItem>();

    // TODO: Warning: This will break the tree view
    //wrapper->style()->minHeight = ReflowUnit { ReflowUnitType::eAbsolute, row_min_height };
    //wrapper->style()->margin = Margin { static_cast<float>(view_.depth) * ident_per_level, 0.F, 0.F, 0.F };
    static_cast<ptr<HorizontalPanel>>(wrapper.get())->attr.style.setValue(
        PanelStyle {
            .backgroundColor = engine::color { 255.F, 255.F, 255.F, 0.F }
        }
    );
    static_cast<ptr<HorizontalPanel>>(wrapper.get())->attr.minHeight.setValue(
        { ReflowUnitType::eAbsolute, row_min_height }
    );
    wrapper->attr.level.setValue(view_.depth);

    wrapper->addChild(content_);
    return wrapper;
}

void TreeViewBase::changeStateSelection(cref<sptr<Widget>> item_, const bool selected_) {
    static_cast<const ptr<TreeItem>>(item_.get())->setSelected(selected_);
}

EventResponse TreeViewBase::onFocus(cref<FocusEvent> event_) {
    _state |= WidgetStateFlagBits::eFocus;
    markAsPending();

    return VScrollBox::onFocus(event_);
}

EventResponse TreeViewBase::onBlur(cref<FocusEvent> event_) {
    _state.unwrap &= (~static_cast<WidgetState::value_type>(WidgetStateFlagBits::eFocus));
    markAsPending();

    return VScrollBox::onBlur(event_);
}

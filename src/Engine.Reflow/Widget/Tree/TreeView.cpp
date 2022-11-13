#include "TreeView.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "TreeItem.hpp"
#include "../../Style/BoundStyleSheet.hpp"
#include "../../Style/StyleSheet.hpp"
#include "../HBox.hpp"

using namespace ember::engine::reflow;
using namespace ember;

static constexpr float ident_per_level = 12.F;
static constexpr float row_min_height = 20.F;

sptr<Widget> TreeViewBase::generateRow(cref<TreeViewItem> view_, cref<sptr<Widget>> content_) {

    auto wrapper = make_sptr<TreeItem>();

    wrapper->style()->minHeight = ReflowUnit { ReflowUnitType::eAbsolute, row_min_height };
    wrapper->style()->margin = Margin { static_cast<float>(view_.depth) * ident_per_level, 0.F, 0.F, 0.F };

    wrapper->addChild(content_);
    return wrapper;
}

void TreeViewBase::changeStateSelection(cref<sptr<Widget>> item_, const bool selected_) {
    static_cast<const ptr<TreeItem>>(item_.get())->setSelected(selected_);
}

EventResponse TreeViewBase::onFocus(cref<FocusEvent> event_) {
    _state.focus = true;
    markAsPending();

    return VScrollBox::onFocus(event_);
}

EventResponse TreeViewBase::onBlur(cref<FocusEvent> event_) {
    _state.focus = false;
    markAsPending();

    return VScrollBox::onBlur(event_);
}

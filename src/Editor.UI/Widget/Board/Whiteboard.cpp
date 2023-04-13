#include "Whiteboard.hpp"

#include <Engine.Common/Make.hpp>
#include "../../Color/Dark.hpp"
#include "Engine.Reflow/Layout/Style.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

Whiteboard::Whiteboard() :
    Panel(),
    attr(
        Attributes {
            .minWidth = { this, { ReflowUnitType::eRelative, 1.F } },
            .width = { this, { ReflowUnitType::eRelative, 1.F } },
            .maxWidth = { this, { ReflowUnitType::eRelative, 1.F } },
            .minHeight = { this, { ReflowUnitType::eRelative, 1.F } },
            .height = { this, { ReflowUnitType::eRelative, 1.F } },
            .maxHeight = { this, { ReflowUnitType::eRelative, 1.F } },
            .flexGrow = { this, 1.F },
            .flexShrink = { this, 1.F }
        }
    ) {}

Whiteboard::~Whiteboard() = default;

string Whiteboard::getTag() const noexcept {
    return _STD format(R"(Whiteboard <{:#x}>)", reinterpret_cast<u64>(this));
}

EventResponse Whiteboard::onFocus(cref<FocusEvent> event_) {

    state() |= WidgetStateFlagBits::eFocus;
    // Warning: Temporary Fix
    markAsPending();

    return Widget::onFocus(event_);
}

EventResponse Whiteboard::onBlur(cref<FocusEvent> event_) {

    state().unset(WidgetStateFlagBits::eFocus);
    // Warning: Temporary Fix
    markAsPending();

    return Widget::onBlur(event_);
}

const non_owning_rptr<const Children> Whiteboard::children() const {
    return &_children;
}

sptr<Board> Whiteboard::getBoard() const noexcept {
    if (_children.empty()) {
        return {};
    }
    return _STD static_pointer_cast<Board, Widget>(_children[0]);
}

void Whiteboard::setBoard(cref<sptr<Widget>> board_) {
    if (_children.empty()) {
        board_->setParent(shared_from_this());
        _children.push_back(board_);
        return;
    }

    _children[0]->setParent(nullptr);

    board_->setParent(shared_from_this());
    _children[0] = board_;
}

sptr<Widget> Whiteboard::getOverlay() const noexcept {
    if (_children.size() < 2) {
        return {};
    }
    return _children[1];
}

void Whiteboard::setOverlay(cref<sptr<Widget>> overlay_) {
    assert(not _children.empty());

    if (_children.size() < 2) {
        _children.push_back(overlay_);
        return;
    }

    _children[1] = overlay_;
}

math::vec2 Whiteboard::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {

    math::vec2 size = math::vec2 { 0.F };

    /**/

    if (attr.width->type == ReflowUnitType::eAbsolute) {
        size.x = attr.width->value;
    }
    if (attr.height->type == ReflowUnitType::eAbsolute) {
        size.y = attr.height->value;
    }

    /**/

    return layout::clampSizeAbs(attr, size);
}

math::vec2 Whiteboard::computeDesiredSize(
    cref<ReflowPassState> passState_
) const {

    math::vec2 desired { getDesiredSize() };
    if (attr.width->type == ReflowUnitType::eRelative) {
        desired.x = passState_.referenceSize.x * attr.width->value;
    }
    if (attr.height->type == ReflowUnitType::eRelative) {
        desired.y = passState_.referenceSize.y * attr.height->value;
    }

    return layout::clampSize(
        attr,
        passState_.layoutSize,
        desired
    );
}

void Whiteboard::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {
    for (const auto& child : *children()) {
        const auto state = state_.getStateOf(child);
        state->layoutOffset = ctx_.localOffset;
        state->layoutSize = ctx_.localSize;
    }
}

bool Whiteboard::willChangeLayout(cref<math::vec2> space_) const noexcept {

    if (_state.isProxyPending()) {
        return true;
    }

    return Widget::willChangeLayout(space_);
}

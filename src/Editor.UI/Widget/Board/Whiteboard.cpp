#include "Whiteboard.hpp"

#include "../../Color/Dark.hpp"
#include "Engine.Reflow/Layout/Constraints.hpp"
#include <Engine.Reflow/Algorithm/Fragments.hpp>

#include "Editor.UI/Style/Style.hpp"
#include "Engine.Reflow/Style/StyleCondition.hpp"
#include <Engine.Common/Make.hpp>

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

static sptr<BoundStyleSheet> makeWhiteboardStyle() {

    auto style = BoundStyleSheet::make(
        StyleSheet {
            .minWidth = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .maxWidth = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .minHeight = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .height = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .maxHeight = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            /**/
            .border = { true, Padding { 1.F } },
            .borderColor = { true, color::Dark::grey },
            /**/
            .color = { true, color::Dark::backgroundDefault },
        }
    );

    style->pushStyle(
        StyleChainEntry {
            Style::key_type::from("Whiteboard::Focused"),
            style::isFocused,
            make_sptr<StyleSheet>(
                StyleSheet {
                    .borderColor = { true, color::Dark::darkRed }
                }
            )
        }
    );

    return style;
}

Whiteboard::Whiteboard() :
    Widget(),
    _style(makeWhiteboardStyle()) {}

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

cref<sptr<BoundStyleSheet>> Whiteboard::style() const noexcept {
    return _style;
}

ref<sptr<BoundStyleSheet>> Whiteboard::style() noexcept {
    return _style;
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
        _children.push_back(board_);
        return;
    }
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

void Whiteboard::render(const ptr<ReflowCommandBuffer> cmd_) {

    const math::vec2 off { screenOffset() + math::vec2 { _computedStyle.margin.attr.x, _computedStyle.margin.attr.y } };
    const math::vec2 size {
        innerSize() + math::vec2 {
            _computedStyle.padding.attr.x + _computedStyle.padding.attr.z,
            _computedStyle.padding.attr.y + _computedStyle.padding.attr.w
        }
    };

    if (_computedStyle.borderRadius.attr.zero()) {

        const math::vec2 c0 { off.x, off.y };
        const math::vec2 c1 { off.x + size.x, off.y };
        const math::vec2 c2 { off.x + size.x, off.y + size.y };
        const math::vec2 c3 { off.x, off.y + size.y };

        cmd_->drawQuad(c0, c1, c2, c3, _computedStyle.color.attr);

    } else {

        /**
         *
         */
        math::vec2 c0 { off.x + _computedStyle.borderRadius.attr.x, off.y + _computedStyle.borderRadius.attr.x };
        math::vec2 c1 {
            (off.x + size.x) - _computedStyle.borderRadius.attr.y,
            off.y + _computedStyle.borderRadius.attr.y
        };
        math::vec2 c2 {
            (off.x + size.x) - _computedStyle.borderRadius.attr.z,
            (off.y + size.y) - _computedStyle.borderRadius.attr.z
        };
        math::vec2 c3 {
            off.x + _computedStyle.borderRadius.attr.w,
            (off.y + size.y) - _computedStyle.borderRadius.attr.w
        };

        cmd_->drawQuad(c0, c1, c2, c3, _computedStyle.color.attr);

        /**
         *
         */
        math::vec2 c00 { off.x + _computedStyle.borderRadius.attr.x, off.y };
        math::vec2 c01 { (off.x + size.x) - _computedStyle.borderRadius.attr.y, off.y };
        math::vec2 c02 {
            (off.x + size.x) - _computedStyle.borderRadius.attr.y,
            off.y + _computedStyle.borderRadius.attr.y
        };
        math::vec2 c03 { off.x + _computedStyle.borderRadius.attr.x, off.y + _computedStyle.borderRadius.attr.x };

        cmd_->drawQuad(c00, c01, c02, c03, _computedStyle.color.attr);

        /**
         *
         */
        math::vec2 c10 { off.x + size.x, off.y + _computedStyle.borderRadius.attr.y };
        math::vec2 c11 { off.x + size.x, (off.y + size.y) - _computedStyle.borderRadius.attr.z };
        math::vec2 c12 {
            (off.x + size.x) - _computedStyle.borderRadius.attr.z,
            (off.y + size.y) - _computedStyle.borderRadius.attr.z
        };
        math::vec2 c13 {
            (off.x + size.x) - _computedStyle.borderRadius.attr.y,
            off.y + _computedStyle.borderRadius.attr.y
        };

        cmd_->drawQuad(c10, c11, c12, c13, _computedStyle.color.attr);

        /**
         *
         */
        math::vec2 c20 { (off.x + size.x) - _computedStyle.borderRadius.attr.z, off.y + size.y };
        math::vec2 c21 { off.x + _computedStyle.borderRadius.attr.w, off.y + size.y };
        math::vec2 c22 {
            off.x + _computedStyle.borderRadius.attr.w,
            (off.y + size.y) - _computedStyle.borderRadius.attr.w
        };
        math::vec2 c23 {
            (off.x + size.x) - _computedStyle.borderRadius.attr.z,
            (off.y + size.y) - _computedStyle.borderRadius.attr.z
        };

        cmd_->drawQuad(c20, c21, c22, c23, _computedStyle.color);

        /**
         *
         */
        math::vec2 c30 { off.x, (off.y + size.y) - _computedStyle.borderRadius.attr.w };
        math::vec2 c31 { off.x, off.y + _computedStyle.borderRadius.attr.x };
        math::vec2 c32 { off.x + _computedStyle.borderRadius.attr.x, off.y + _computedStyle.borderRadius.attr.x };
        math::vec2 c33 {
            off.x + _computedStyle.borderRadius.attr.w,
            (off.y + size.y) - _computedStyle.borderRadius.attr.w
        };

        cmd_->drawQuad(c30, c31, c32, c33, _computedStyle.color.attr);

        /**
         *
         */
        cmd_->drawArc(
            math::vec2 {
                off.x + _computedStyle.borderRadius.attr.x,
                off.y + _computedStyle.borderRadius.attr.x
            },
            _computedStyle.borderRadius.attr.x,
            glm::radians(180.F),
            glm::radians(270.F),
            _computedStyle.color.attr
        );
        cmd_->drawArc(
            math::vec2 {
                off.x + size.x - _computedStyle.borderRadius.attr.y,
                off.y + _computedStyle.borderRadius.attr.y
            },
            _computedStyle.borderRadius.attr.y,
            glm::radians(270.F),
            glm::radians(360.F),
            _computedStyle.color.attr
        );
        cmd_->drawArc(
            math::vec2 {
                off.x + size.x - _computedStyle.borderRadius.attr.z,
                off.y + size.y - _computedStyle.borderRadius.attr.z
            },
            _computedStyle.borderRadius.attr.z,
            glm::radians(0.F),
            glm::radians(90.F),
            _computedStyle.color.attr
        );
        cmd_->drawArc(
            math::vec2 {
                off.x + _computedStyle.borderRadius.attr.w,
                off.y + size.y - _computedStyle.borderRadius.attr.w
            },
            _computedStyle.borderRadius.attr.w,
            glm::radians(90.F),
            glm::radians(180.F),
            _computedStyle.color.attr
        );

    }

    /* TODO: Relocate */
    for (const auto& child : *children()) {
        if (child->state().isVisible() && not cmd_->scissorCull(child->screenOffset(), child->outerSize())) {
            child->render(cmd_);
        }
    }
}

void Whiteboard::flow(
    cref<FlowContext> ctx_,
    cref<math::vec2> space_,
    cref<math::vec2> limit_,
    ref<StyleKeyStack> styleStack_
) {

    styleStack_.pushLayer();
    _computedStyle = _style->compute(shared_from_this(), styleStack_);

    /**/

    math::vec2 local { algorithm::calcImplicitInnerSize(_computedStyle, space_, limit_) };
    algorithm::applyPaddingToOuter(_computedStyle, local);

    /**/

    for (const auto& widget : *children()) {

        constexpr math::vec2 limit { _STD numeric_limits<float>::infinity() };

        if (widget->willChangeLayout(local, styleStack_)) {
            widget->flow(ctx_, local, limit, styleStack_);
        }

    }

    /**
     * Content State
     */
    _innerSize = local;
    styleStack_.popLayer();

    /**
     * State Changes
     */
    clearPending();
    markCaptureState();

    _prevSpace = space_;
    _prevStyleStack.clear();
    styleStack_.compress(_prevStyleStack);
}

void Whiteboard::shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) {

    if (_screenOff != offset_) {
        markCaptureState();
    }

    _screenOff = offset_;

    /**/

    math::vec2 localOffset { offset_ };

    if (!_computedStyle.margin->zero()) {
        localOffset.x += _computedStyle.margin->x;
        localOffset.y += _computedStyle.margin->y;
    }

    if (!_computedStyle.padding->zero()) {
        localOffset.x += _computedStyle.padding->x;
        localOffset.y += _computedStyle.padding->y;
    }

    for (const auto& widget : *children()) {
        widget->shift(ctx_, localOffset);
    }

    /**/

    clearShiftState();
}

math::vec2 Whiteboard::outerSize() const noexcept {

    auto size { innerSize() };

    if (not _computedStyle.padding.attr.zero()) {
        size.x += _computedStyle.padding.attr.x;
        size.x += _computedStyle.padding.attr.z;

        size.y += _computedStyle.padding.attr.y;
        size.y += _computedStyle.padding.attr.w;
    }

    if (not _computedStyle.border.attr.zero()) {
        size.x += _computedStyle.border.attr.x;
        size.x += _computedStyle.border.attr.z;

        size.y += _computedStyle.border.attr.y;
        size.y += _computedStyle.border.attr.w;
    }

    if (not _computedStyle.margin.attr.zero()) {
        size.x += _computedStyle.margin.attr.x;
        size.x += _computedStyle.margin.attr.z;

        size.y += _computedStyle.margin.attr.y;
        size.y += _computedStyle.margin.attr.w;
    }

    return size;
}

math::vec2 Whiteboard::innerSize() const noexcept {
    return _innerSize;
}

math::vec2 Whiteboard::screenOffset() const noexcept {
    return _screenOff;
}

bool Whiteboard::willChangeLayout(
    cref<math::vec2> space_,
    cref<StyleKeyStack> styleStack_
) const noexcept {

    if (_state.isProxyPending()) {
        return true;
    }

    if (layout::hasStyleChanged(_prevStyleStack, styleStack_)) {
        return true;
    }

    if (_prevSpace != space_) {
        return not layout::hasConstSize(_computedStyle);
    }

    return Widget::willChangeLayout(space_, styleStack_);
}

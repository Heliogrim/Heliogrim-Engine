#include "Board.hpp"

#include <Engine.Reflow/Algorithm/Fragments.hpp>
#include <Engine.Reflow/Layout/Constraints.hpp>

#include "Editor.UI/Color/Dark.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

Board::Board() :
    Widget(),
    _style(
        BoundStyleSheet::make(
            StyleSheet {
                .minWidth = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .maxWidth = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .minHeight = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .height = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .color = { true, color::Dark::backgroundInnerFieldDarken }
            }
        )
    ) {}

Board::~Board() = default;

string Board::getTag() const noexcept {
    return _STD format(R"(Board <{:#x}>)", reinterpret_cast<u64>(this));
}

EventResponse Board::onFocus(cref<FocusEvent> event_) {

    state() |= WidgetStateFlagBits::eFocus;
    // Warning: Temporary Fix
    markAsPending();

    return Widget::onFocus(event_);
}

EventResponse Board::onBlur(cref<FocusEvent> event_) {

    _expMouseHold = false;

    state().unset(WidgetStateFlagBits::eFocus);
    // Warning: Temporary Fix
    markAsPending();

    return Widget::onBlur(event_);
}

EventResponse Board::onMouseButtonDown(cref<MouseEvent> event_) {

    if (event_._button == 0x1) {
        _expMouseHold = true;
    }

    return Widget::onMouseButtonDown(event_);
}

EventResponse Board::onMouseButtonUp(cref<MouseEvent> event_) {

    if (event_._button == 0x1) {
        _expMouseHold = false;
    }

    return Widget::onMouseButtonUp(event_);
}

EventResponse Board::onMouseMove(cref<MouseMoveEvent> event_) {

    if (_expMouseHold) {

        const auto speed = math::vec2 { 1.F, 1.F };
        const auto delta = math::vec2(event_._delta) * speed;

        _offCenter += delta;

        markAsPending();
        return EventResponse::eConsumed;
    }

    return Widget::onMouseMove(event_);
}

EventResponse Board::onWheel(cref<WheelEvent> event_) {

    constexpr float zoomSpeed = 0.1F;
    const auto delta = event_._value.y * zoomSpeed;

    _offCenter.z += delta;
    _offCenter.z = MAX(_offCenter.z, 0.001F);
    _offCenter.z = MIN(_offCenter.z, 1000.F);

    return EventResponse::eConsumed;
}

EventResponse Board::onKeyDown(cref<KeyboardEvent> event_) {

    EventResponse response;
    switch (event_._key) {
        case /*SDL_SCANCODE_LEFT*/80: {
            // Left
            _offCenter.x += 16.F;
            response = EventResponse::eConsumed;
            break;
        }
        case /*SDL_SCANCODE_UP*/82: {
            // Up
            _offCenter.y += 16.F;
            response = EventResponse::eConsumed;
            break;
        }
        case /*SDL_SCANCODE_RIGHT*/79: {
            // Right
            _offCenter.x -= 16.F;
            response = EventResponse::eConsumed;
            break;
        }
        case /*SDL_SCANCODE_DOWN*/81: {
            // Down
            _offCenter.y -= 16.F;
            response = EventResponse::eConsumed;
            break;
        }
        default: {
            return Widget::onKeyDown(event_);
        }
    }

    return response;
}

EventResponse Board::onKeyUp(cref<KeyboardEvent> event_) {
    return Widget::onKeyUp(event_);
}

const ptr<const Children> Board::children() const {
    return &_children;
}

void Board::addChild(cref<sptr<Widget>> child_) {
    return _children.push_back(child_);
}

void Board::render(const ptr<ReflowCommandBuffer> cmd_) {

    /**
     * TODO: Generate background image like "Graph Format" of Windows Whiteboard -> Adaptive Grid-Lines
     * TODO: We might need support for shader code to maintain performant display of certain features
     */

    const auto inner = innerSize();
    const math::vec2 off = screenOffset() + math::vec2 { _computedStyle.margin.attr.x, _computedStyle.margin.attr.y };

    const auto scissor = math::fExtent2D {
        inner.x, inner.y,
        off.x, off.y
    };
    cmd_->pushIntersectScissor(scissor);

    /**/

    const auto drawRectSize = inner + math::vec2 {
        _computedStyle.padding.attr.x + _computedStyle.padding.attr.z,
        _computedStyle.padding.attr.y + _computedStyle.padding.attr.w
    };

    const math::vec2 c0 { off.x, off.y };
    const math::vec2 c1 { off.x + drawRectSize.x, off.y };
    const math::vec2 c2 { off.x + drawRectSize.x, off.y + drawRectSize.y };
    const math::vec2 c3 { off.x, off.y + drawRectSize.y };

    cmd_->drawQuad(c0, c1, c2, c3, _computedStyle.color.attr);

    /**/

    /* TODO: Relocate */
    for (const auto& child : *children()) {
        if (child->state().isVisible() && not cmd_->scissorCull(child->screenOffset(), child->outerSize())) {
            child->render(cmd_);
        }
    }

    /**/

    cmd_->popScissor();
}

void Board::flow(
    cref<FlowContext> ctx_,
    cref<math::vec2> space_,
    cref<math::vec2> limit_,
    ref<StyleKeyStack> styleStack_
) {

    styleStack_.pushLayer();
    _computedStyle = _style->compute(shared_from_this(), styleStack_);

    /**/

    math::vec2 local = algorithm::calcImplicitInnerSize(_computedStyle, space_, limit_);
    // algorithm::applyPaddingToOuter(_computedStyle, local); // Board should ignore padding

    /**/

    for (const auto& widget : *children()) {

        constexpr math::vec2 limit { _STD numeric_limits<float>::infinity() };

        if (widget->willChangeLayout(local, styleStack_)) {
            widget->flow(ctx_, local, limit, styleStack_);
        }
    }

    /**/

    // We don't accommodate for overflow sizes, cause board is meant be have infinite space

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

void Board::shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) {

    if (_screenOff != offset_ || _prevOffCenter != _offCenter) {
        markCaptureState();
    }

    _screenOff = offset_;
    _prevOffCenter = _offCenter;

    /**/

    const auto halfSize = _innerSize / 2.F;
    const math::vec2 offset = offset_ + _offCenter + halfSize;

    for (const auto& widget : *children()) {

        /**
         * We assume that all direct children of board should be handles with absolute positioning
         *  Note: Children should use there margin style parameter to define the offset to the board center.
         */

        widget->shift(ctx_, offset);
    }

    /**/

    clearShiftState();
}

math::vec2 Board::outerSize() const noexcept {

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

math::vec2 Board::innerSize() const noexcept {
    return _innerSize;
}

math::vec2 Board::screenOffset() const noexcept {
    return _screenOff;
}

math::vec2 Board::offCenter() const noexcept {
    return _offCenter;
}

float Board::zoomFactor() const noexcept {
    return _offCenter.z;
}

bool Board::willChangeLayout(cref<math::vec2> space_, cref<StyleKeyStack> styleStack_) const noexcept {

    if (_state.isProxyPending()) {
        return true;
    }

    if (layout::hasStyleChanged(_prevStyleStack, styleStack_)) {
        return true;
    }

    if (_prevOffCenter != _offCenter) {
        return true;
    }

    if (_prevSpace != space_) {
        return not layout::hasConstSize(_computedStyle);
    }

    return Widget::willChangeLayout(space_, styleStack_);
}

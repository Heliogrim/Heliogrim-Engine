#include "Board.hpp"

#include <Engine.Reflow/Layout/Style.hpp>

#include "Editor.UI/Color/Dark.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

Board::Board() :
    Widget(),
    attr(
        Attributes {
            .minWidth = { this, { ReflowUnitType::eRelative, 1.F } },
            .width = { this, { ReflowUnitType::eRelative, 1.F } },
            .maxWidth = { this, { ReflowUnitType::eRelative, 1.F } },
            .minHeight = { this, { ReflowUnitType::eRelative, 1.F } },
            .height = { this, { ReflowUnitType::eRelative, 1.F } },
            .maxHeight = { this, { ReflowUnitType::eRelative, 1.F } }
        }
    ),
    _expMouseHold(false) {}

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
    child_->setParent(shared_from_this());
    return _children.push_back(child_);
}

void Board::render(const ptr<ReflowCommandBuffer> cmd_) {

    /**
     * TODO: Generate background image like "Graph Format" of Windows Whiteboard -> Adaptive Grid-Lines
     * TODO: We might need support for shader code to maintain performant display of certain features
     */

    const auto offset = _layoutState.layoutOffset;
    const auto size = _layoutState.layoutSize;

    const auto scissor = math::fExtent2D {
        size.x, size.y,
        offset.x, offset.y
    };
    cmd_->pushIntersectScissor(scissor);

    /**/

    const math::vec2 c0 { offset.x, offset.y };
    const math::vec2 c1 { offset.x + size.x, offset.y };
    const math::vec2 c2 { offset.x + size.x, offset.y + size.y };
    const math::vec2 c3 { offset.x, offset.y + size.y };

    // TODO:
    cmd_->drawQuad(c0, c1, c2, c3, color::Dark::backgroundInnerFieldDarken);

    /**/

    /* TODO: Relocate */
    for (const auto& child : *children()) {
        if (child->state().isVisible() && not cmd_->scissorCull(
            child->layoutState().layoutOffset,
            child->layoutState().layoutSize
        )) {
            child->render(cmd_);
        }
    }

    /**/

    cmd_->popScissor();
}

math::vec2 Board::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {

    math::vec2 desired { 0.F };
    if (attr.width->type == ReflowUnitType::eAbsolute) {
        desired.x = attr.width->value;
    }
    if (attr.height->type == ReflowUnitType::eAbsolute) {
        desired.y = attr.height->value;
    }

    return layout::clampSizeAbs(attr, desired);
}

math::vec2 Board::computeDesiredSize(
    cref<ReflowPassState> passState_
) const {

    math::vec2 desired = getDesiredSize();
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

#include "BoardNode.hpp"

void Board::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {

    const auto halfSize = ctx_.localSize / 2.F;
    const math::vec2 offset = ctx_.localOffset + halfSize + _offCenter;

    for (const auto& child : *children()) {

        // Warning: Unsafe
        const auto node = _STD static_pointer_cast<BoardNode>(child);

        // TODO: We need to recalc the offset per child to apply the transformation offset
        // TODO: Check whether we should care about the external layout size, cause graph should work absolute
        const auto state = state_.getStateOf(child);
        state->layoutOffset = offset + node->getBoardPosition();
        state->layoutSize = child->getDesiredSize();
    }

}

math::vec2 Board::offCenter() const noexcept {
    return _offCenter;
}

float Board::zoomFactor() const noexcept {
    return _offCenter.z;
}

bool Board::willChangeLayout(cref<math::vec2> space_) const noexcept {

    if (_state.isProxyPending()) {
        return true;
    }

    if (_prevOffCenter != _offCenter) {
        return true;
    }

    return Widget::willChangeLayout(space_);
}

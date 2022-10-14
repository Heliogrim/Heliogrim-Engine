#include "Widget.hpp"

#include "Engine.Input/MouseButtonEvent.hpp"
#include "Engine.Common/Exception/NotImplementedException.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

Widget::Widget() :
    _parent(nullptr),
    _nodes(),
    _state(),
    _margin(),
    _reflowType(ReflowType::eFlexRow),
    _reflowSpacing(ReflowSpacing::eSpaceAround),
    _emitter() {}

math::vec2 Widget::flow(cref<UIContext> ctx_, cref<math::vec2> available_) {
    return Reflow::flow(this, ctx_, available_);
}

void Widget::shift(cref<UIContext> ctx_, cref<math::vec2> offset_) {
    Reflow::shift(this, ctx_, offset_);
}

ptr<Widget> Widget::parent() const noexcept {
    return _parent;
}

void Widget::setParent(const non_owning_rptr<Widget> parent_) {
    _parent = parent_;
}

void Widget::add(mref<ptr<Widget>> widget_) {
    //widget_->setParent(this);
    throw NotImplementedException();
}

void Widget::add(cref<sptr<Widget>> widget_) {
    widget_->setParent(this);
    _nodes.push_back(widget_);
}

bool Widget::has(const ptr<Widget> widget_) {
    throw NotImplementedException();
}

bool Widget::has(cref<sptr<Widget>> widget_) {
    return _STD find(_nodes.begin(), _nodes.end(), widget_) != _nodes.end();
}

void Widget::remove(const ptr<Widget> widget_) {
    //widget_->setParent(nullptr);
    throw NotImplementedException();
}

void Widget::remove(cref<sptr<Widget>> widget_) {
    widget_->setParent(nullptr);
    const auto it { _STD remove(_nodes.begin(), _nodes.end(), widget_) };
    _nodes.erase(it, _nodes.end());
}

bool Widget::visible() const noexcept {
    return _state.visible;
}

void Widget::setVisible(bool visible_) {
    _state.visible = visible_;
}

bool Widget::recursiveVisible() const noexcept {
    if (_parent && _state.visible) {
        return _parent->recursiveVisible();
    }
    return _state.visible;
}

bool Widget::focused() const noexcept {
    return _state.focused;
}

bool Widget::hovered() const noexcept {
    return _state.hover;
}

cref<math::fExtent2D> Widget::transform() const noexcept {
    return _transform;
}

bool Widget::contains(cref<math::ivec2> point_) const noexcept {
    const math::vec2 diff {
        static_cast<float>(point_.x) - _transform.offsetX,
        static_cast<float>(point_.y) - _transform.offsetY
    };

    return diff.x >= 0 && _transform.width > diff.x &&
        diff.y >= 0 && _transform.height > diff.y;
}

ReflowType Widget::reflowType() const noexcept {
    return _reflowType;
}

void Widget::setReflowType(const ReflowType type_) {
    _reflowType = type_;
}

ReflowSpacing Widget::reflowSpacing() const noexcept {
    return _reflowSpacing;
}

void Widget::setReflowSpacing(const ReflowSpacing spacing_) {
    _reflowSpacing = spacing_;
}

ReflowWrapping Widget::reflowWrapping() const noexcept {
    return _reflowWrapping;
}

void Widget::setReflowWrapping(const ReflowWrapping wrapping_) {
    _reflowWrapping = wrapping_;
}

ReflowOverflow Widget::reflowOverflow() const noexcept {
    return _reflowOverflow;
}

void Widget::setReflowOverflow(const ReflowOverflow overflow_) {
    _reflowOverflow = overflow_;
}

cref<WidgetEventEmitter> Widget::emitter() const noexcept {
    return _emitter;
}

ref<WidgetEventEmitter> Widget::emitter() noexcept {
    return _emitter;
}

bool Widget::onMouseButtonEvent(cref<math::ivec2> pointer_, u32 button_, bool down_, u32 modifier_) {

    _emitter.emit(input::event::MouseButtonEvent { pointer_, button_, down_, modifier_ });

    bool result { false };
    for (auto rit { _nodes.rbegin() }; rit != _nodes.rend(); ++rit) {

        auto& widget { *rit };
        if (widget->visible() && widget->contains(pointer_)) {
            widget->onMouseButtonEvent(pointer_, button_, down_, modifier_);
            result = true;
            break;
        }

    }

    return result;
}

bool Widget::onMouseMotionEvent(cref<math::ivec2> pointer_, cref<math::ivec2> delta_, u32 button_, u32 modifier_) {

    for (auto rit { _nodes.rbegin() }; rit != _nodes.rend(); ++rit) {

        auto& widget { *rit };
        if (!widget->visible()) {
            continue;
        }

        bool prevCont = widget->contains(pointer_ - delta_);
        bool cont = widget->contains(pointer_);

        if (prevCont != cont) {
            widget->onMouseEnterEvent(pointer_, cont);
        }

        if ((prevCont || cont) && widget->onMouseMotionEvent(pointer_, delta_, button_, modifier_)) {
            // Warning: Might result in missing leave events when container has dismounted elements / popover
            return true;
        }
    }

    return true;
}

bool Widget::onMouseDragEvent(cref<math::ivec2> pointer_, cref<math::ivec2> delta_, u32 button_, u32 modifier_) {
    return false;
}

bool Widget::onMouseEnterEvent(cref<math::ivec2> pointer_, bool enter_) {
    _state.hover = enter_;
    return true;
}

bool Widget::onFocusEvent(bool focus_) {
    _state.focused = focus_;
    return true;
}

bool Widget::onKeyboardEvent(u32 key_, u32 action_, u32 modifier_) {
    return false;
}

bool Widget::onKeyboardCharEvent(u32 code_) {
    return false;
}

StatefulWidget::StatefulWidget() = default;

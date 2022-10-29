#include "Widget.hpp"

#include "Engine.Input/MouseButtonEvent.hpp"
#include "Engine.Common/Exception/NotImplementedException.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

Widget::Widget() :
    _parent(nullptr),
    _nodes(),
    _state(),
    _reflowPosition(ReflowPosition::eStatic),
    _margin(),
    _padding(),
    _reflowType(ReflowType::eFlexRow),
    _reflowSpacing(ReflowSpacing::eSpaceAround),
    _reflowWrapping(ReflowWrapping::eNone),
    _reflowOverflow(ReflowOverflow::eNone),
    _emitter() {}

math::vec2 Widget::flow(cref<UIContext> ctx_, cref<math::vec2> available_) {

    auto it {
        _STD ranges::remove(_nodes.begin(), _nodes.end(), true, [](cref<sptr<Widget>> node_) {
            return node_->markedAsDeleted();
        })
    };
    _nodes.erase(it.begin(), it.end());

    /**/

    return Reflow::flow(this, ctx_, available_);
}

void Widget::shift(cref<UIContext> ctx_, cref<math::vec2> offset_) {
    Reflow::shift(this, ctx_, offset_);
}

const non_owning_rptr<const Widget> Widget::root() const noexcept {
    ptr<const Widget> cur { this };
    while (cur->_parent) {
        cur = cur->_parent;
    }

    return cur->_parent ? cur->_parent : cur;
}

const non_owning_rptr<Widget> Widget::root() noexcept {
    ptr<Widget> cur { this };
    while (cur->_parent) {
        cur = cur->_parent;
    }

    return cur->_parent ? cur->_parent : cur;
}

ptr<Widget> Widget::parent() const noexcept {
    return _parent;
}

void Widget::setParent(const non_owning_rptr<Widget> parent_) {
    _parent = parent_;
}

ref<Vector<sptr<Widget>>> Widget::nodes() noexcept {
    return _nodes;
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
    widget_->setParent(nullptr);
    const auto it {
        _STD ranges::remove(_nodes.begin(), _nodes.end(), widget_, [](cref<sptr<Widget>> node_) {
            return node_.get();
        })
    };
    _nodes.erase(it.begin(), it.end());
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

void Widget::requestFocus() {
    root()->updateFocus(this);
}

void Widget::updateFocus(ptr<Widget> widget_) {

    auto next { this };
    while (next) {

        auto cur = next;
        next = nullptr;

        for (const auto& entry : cur->_nodes) {
            if (entry->focused()) {
                entry->onFocusEvent(false);
                next = entry.get();
                break;
            }
        }
    }

    /**/

    next = widget_;
    while (next) {
        next->onFocusEvent(true);
        next = next->parent();
    }
}

bool Widget::hovered() const noexcept {
    return _state.hover;
}

bool Widget::markedAsDeleted() const noexcept {
    return _state.deleted;
}

void Widget::markAsDeleted() noexcept {
    _state.deleted = true;
}

bool Widget::markedAsTouched() const noexcept {
    return _state.touched;
}

void Widget::markAsTouched() noexcept {
    _state.touched = true;
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

engine::color Widget::statedColor() const noexcept {
    return _color;
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

    if (!result && down_ && not focused()) {
        requestFocus();
    }

    return result;
}

bool Widget::onMouseMotionEvent(cref<math::ivec2> pointer_, cref<math::ivec2> delta_, u32 button_, u32 modifier_) {

    bool result { false };
    for (auto rit { _nodes.rbegin() }; rit != _nodes.rend(); ++rit) {

        auto& widget { *rit };
        if (!widget->visible()) {
            continue;
        }

        const bool prevCont = widget->contains(pointer_ - delta_);
        const bool cont = widget->contains(pointer_);

        if (prevCont != cont || (cont && not widget->_state.hover)) {
            widget->onMouseEnterEvent(pointer_, cont);
        }

        if ((prevCont || cont) && widget->onMouseMotionEvent(pointer_, delta_, button_, modifier_)) {
            // Warning: Might result in missing leave events when container has dismounted elements / popover
            result = true;

        } else if (widget->_state.hover) {
            widget->onMouseMotionEvent(pointer_, delta_, button_, modifier_);
            widget->onMouseEnterEvent(pointer_, cont);
        }
    }

    return result;
}

bool Widget::onMouseDragEvent(cref<math::ivec2> pointer_, cref<math::ivec2> delta_, u32 button_, u32 modifier_) {
    return false;
}

bool Widget::onMouseEnterEvent(cref<math::ivec2> pointer_, bool enter_) {
    _state.hover = enter_;
    return true;
}

bool Widget::onDragDropEvent(cref<input::event::DragDropEvent> event_) {

    for (auto rit { _nodes.rbegin() }; rit != _nodes.rend(); ++rit) {

        const auto& widget { *rit };
        if (!widget->visible()) {
            continue;
        }

        if (widget->contains(event_._pointer) && widget->onDragDropEvent(event_)) {
            return true;
        }
    }

    return false;
}

bool Widget::onScrollEvent(cref<math::ivec2> pointer_, cref<math::vec2> value_) {

    for (auto rit { _nodes.rbegin() }; rit != _nodes.rend(); ++rit) {

        const auto& widget { *rit };
        if (!widget->visible()) {
            continue;
        }

        if (widget->contains(pointer_) && widget->onScrollEvent(pointer_, value_)) {
            return true;
        }
    }

    return false;
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

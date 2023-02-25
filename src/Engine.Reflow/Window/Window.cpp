#include "Window.hpp"

#include <algorithm>
#include <Engine.Common/Make.hpp>

#include "../Widget/Popup.hpp"

using namespace ember::engine::reflow;
using namespace ember;

Window::Window() :
    Widget(),
    _type(WindowType::eNormal),
    _resizable(true),
    _closeable(true),
    _style(nullptr),
    _nativeWindow(nullptr) {}

Window::~Window() {
    tidy();
}

string Window::getTag() const noexcept {
    return _STD format(R"(Window <{:#x}>)", reinterpret_cast<u64>(this));
}

void Window::tidy() {

    Vector<uptr<PopupLayer>> tmp {};
    _STD swap(tmp, _popupLayers);

    tmp.clear();
}

void Window::repackChildren() {

    _children.clear();

    if (_titleBar) {
        _children.push_back(_titleBar);
    }

    if (_content) {
        _children.push_back(_content);
    }

    for (auto it { _popupLayers.begin() }; it != _popupLayers.end(); ++it) {
        if ((*it)->getContent()) {
            _children.push_back((*it)->getContent());
        }
    }
}

void Window::setTitleBar(sptr<Widget> titleBar_) {

    titleBar_->setParent(shared_from_this());

    if (_titleBar && _titleBar->parent().get() == this) {
        _titleBar->setParent(nullptr);
    }

    _titleBar = titleBar_;
    repackChildren();
    markAsPending();

}

void Window::setContent(sptr<Widget> content_) {

    content_->setParent(shared_from_this());

    if (_content && _content->parent().get() == this) {
        _content->setParent(nullptr);
    }

    _content = content_;
    repackChildren();
    markAsPending();
}

sptr<Widget> Window::root() const {
    return const_cast<ptr<Window>>(this)->shared_from_this();
}

const ptr<const Children> Window::children() const {
    return &_children;
}

non_owning_rptr<PopupLayer> Window::pushPopLayer(cref<sptr<Popup>> popup_) {

    auto layer {
        make_uptr<PopupLayer>(
            _STD static_pointer_cast<Window, Widget>(shared_from_this()),
            popup_
        )
    };

    _popupLayers.push_back(_STD move(layer));
    repackChildren();

    return _popupLayers.back().get();
}

void Window::dropPopLayer(const non_owning_rptr<PopupLayer> layer_) {

    if (_popupLayers.empty()) {
        return;
    }

    if (layer_ == nullptr) {
        _popupLayers.pop_back();
        repackChildren();
        return;
    }

    const auto where {
        _STD ranges::remove(
            _popupLayers.begin(),
            _popupLayers.end(),
            layer_,
            [](cref<uptr<PopupLayer>> layer_) {
                return layer_.get();
            }
        )
    };

    _popupLayers.erase(where.begin(), where.end());
    repackChildren();
}

cref<FocusPath> Window::getFocusPath() const noexcept {
    return _focus;
}

sptr<Widget> Window::getFocusTarget() const noexcept {

    if (_focus.empty() || _focus.back().expired()) {
        return nullptr;
    }

    return _focus.back().lock();
}

void Window::render(const ptr<ReflowCommandBuffer> cmd_) {
    for (const auto& child : _children) {
        child->render(cmd_);
    }
}

void Window::flow(
    cref<FlowContext> ctx_,
    cref<math::vec2> space_,
    cref<math::vec2> limit_,
    ref<StyleKeyStack> styleStack_
) {

    /* Warning: Replace */
    _clientSize = space_;
    /**/

    /*
    if (_titleBar) {
        _titleBar->flow(ctx_, space_);
    }
     */

    if (_content) {
        _content->flow(ctx_, space_, limit_, styleStack_);
    }

    /**/

    for (auto it { _popupLayers.begin() }; it != _popupLayers.end(); ++it) {

        auto layer { it->get() };
        auto content { layer->getContent() };

        if (!content) {
            continue;
        }

        math::vec2 space { space_ };
        if (layer->hasOverrideSize()) {
            space = layer->getOverrideSize();
        }

        content->flow(ctx_, space, limit_, styleStack_);
    }
}

void Window::shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) {

    math::vec2 off { offset_ };

    /*
    if (_titleBar) {
        _titleBar->shift(ctx_, off);
        off.y += _titleBar->outerSize().y;
    }
     */

    if (_content) {
        _content->shift(ctx_, off);
    }

    /**/

    for (auto it { _popupLayers.begin() }; it != _popupLayers.end(); ++it) {

        auto layer { it->get() };
        auto content { layer->getContent() };

        if (!content) {
            continue;
        }

        content->shift(ctx_, layer->getScreenPos());
    }

}

math::vec2 Window::outerSize() const noexcept {
    return _clientSize;
}

math::vec2 Window::innerSize() const noexcept {
    return _clientSize;
}

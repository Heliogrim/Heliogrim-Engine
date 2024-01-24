#include "Window.hpp"

#include <algorithm>
#include <format>
#include <Engine.Common/Make.hpp>

#include "../Widget/Popup.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Window::Window() :
    Widget(),
    attr(
        Attributes {
            .style = {
                this, WindowStyle {}
            }
        }
    ),
    _nativeWindow(nullptr),
    _type(WindowType::eNormal),
    _resizable(true),
    _closeable(true) {}

Window::~Window() {
    tidy();
}

string Window::getTag() const noexcept {
    return std::format(R"(Window <{:#x}>)", reinterpret_cast<u64>(this));
}

void Window::tidy() {

    Vector<uptr<PopupLayer>> tmp {};
    std::swap(tmp, _popupLayers);

    tmp.clear();
}

void Window::setClientSize(cref<math::vec2> nextClientSize_) {
    _clientSize = nextClientSize_;
}

math::vec2 Window::getClientSize() const noexcept {
    return _clientSize;
}

void Window::setTitleBar(sptr<Widget> titleBar_) {

    titleBar_->setParent(shared_from_this());
    _children.getChild<0>()->setParent(nullptr);
    _children.setChild<0>(titleBar_);

    markAsPending();
}

void Window::setContent(sptr<Widget> content_) {

    content_->setParent(shared_from_this());
    _children.getChild<1>()->setParent(nullptr);
    _children.setChild<1>(content_);

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
            std::static_pointer_cast<Window, Widget>(shared_from_this()),
            popup_
        )
    };

    /**/

    _popupLayers.push_back(std::move(layer));
    _children.emplace_back(popup_);

    /**/

    return _popupLayers.back().get();
}

void Window::dropPopLayer(const non_owning_rptr<PopupLayer> layer_) {

    if (_popupLayers.empty()) {
        return;
    }

    if (layer_ == nullptr) {

        const auto last = std::move(_popupLayers.back());
        _popupLayers.pop_back();

        std::erase_if(
            _children,
            [key = last->getContent().get()](const auto& entry_) {
                return entry_.get() == key;
            }
        );
        return;
    }

    /**/

    std::erase_if(
        _children,
        [key = layer_->getContent().get()](const auto& entry_) {
            return entry_.get() == key;
        }
    );

    const auto where {
        std::ranges::remove(
            _popupLayers.begin(),
            _popupLayers.end(),
            layer_,
            [](cref<uptr<PopupLayer>> layer_) {
                return layer_.get();
            }
        )
    };

    _popupLayers.erase(where.begin(), where.end());
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

    if (attr.style->backgroundColor.a > 0.F) {

        const math::vec2& offset = _layoutState.layoutOffset;
        const math::vec2& size = _layoutState.layoutSize;

        const math::vec2 c0 { offset.x, offset.y };
        const math::vec2 c1 { offset.x + size.x, offset.y };
        const math::vec2 c2 { offset.x + size.x, offset.y + size.y };
        const math::vec2 c3 { offset.x, offset.y + size.y };

        cmd_->drawQuad(c0, c1, c2, c3, attr.style->backgroundColor);
    }

    /**/

    for (const auto& child : _children) {
        child->render(cmd_);
    }

    for (const auto& layer : _popupLayers) {
        layer->getContent()->render(cmd_);
    }
}

math::vec2 Window::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {
    return _clientSize;
}

math::vec2 Window::computeDesiredSize(cref<ReflowPassState> passState_) const {
    return passState_.referenceSize;
}

void Window::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {

    // TODO: Implement
    for (const auto& child : *children()) {

        const auto state = state_.getStateOf(child);
        state->layoutOffset = ctx_.localOffset;
        state->layoutSize = ctx_.localSize;
    }

    for (const auto& layer : _popupLayers) {

        const auto child = layer->getContent();
        const auto state = state_.getStateOf(child);
        state->layoutOffset = ctx_.localOffset;
        state->layoutSize = ctx_.localSize;
    }
}

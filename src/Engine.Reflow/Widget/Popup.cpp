#include "Popup.hpp"

#include "../Window/Window.hpp"
#include "../Window/PopupLayer.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Popup::Popup() :
    Widget(),
    _layer(nullptr) {}

Popup::~Popup() = default;

void Popup::setPopupLayer(const non_owning_rptr<PopupLayer> layer_) {
    _layer = layer_;
}

non_owning_rptr<PopupLayer> Popup::getPopupLayer() const noexcept {
    return _layer;
}

sptr<Widget> Popup::root() const {
    if (not _layer) {
        return nullptr;
    }

    return _STD static_pointer_cast<Widget, Window>(_layer->getWindow());
}

ReflowPosition Popup::position() const noexcept {
    return ReflowPosition::eAbsolute;
}

math::vec2 Popup::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {

    // TODO: Implement
    // Maybe overridden screen size?
    if (not _layer) {
        return math::vec2 {};
    }
    return _layer->getOverrideSize();
}

void Popup::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {

    // TODO: Implement
    if (not _layer) {
        return;
    }

    _layer->setScreenPos(ctx_.localOffset);
    _layer->overrideScreenSize(ctx_.localSize);// Not intended
}

#include "ScrollContainer.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

ScrollContainer::ScrollContainer():
    ReflowContainer(),
    _scrollTrackColor(46.F, 50.F, 58.F, 255.F),
    _scrollThumbColor(165.F, 165.F, 165.F, 255.F) {}

void ScrollContainer::render(const ptr<UICommandBuffer> cmd_) {

    /**
     *
     */
    const auto& tf { _transform };

    const math::fExtent2D scissor { tf };
    cmd_->pushScissor(scissor);

    /**/
    ReflowContainer::render(cmd_);

    /**/
    const math::vec2 scrollExtent { 8.F, tf.height - 1.F };
    const math::vec2 scrollOrigin { tf.offsetX + tf.width - scrollExtent.x, tf.offsetY };

    const math::vec2 thumbExtent { 8.F, 48.F };
    const math::vec2 scrollOffset { (scrollExtent - thumbExtent) * _scrollValue };

    cmd_->drawQuad(
        scrollOrigin,
        scrollOrigin + math::vec2 { scrollExtent.x, 0.F },
        scrollOrigin + scrollExtent,
        scrollOrigin + math::vec2 { 0.F, scrollExtent.y },
        _scrollTrackColor
    );

    cmd_->drawQuad(
        scrollOrigin + scrollOffset,
        scrollOrigin + scrollOffset + math::vec2 { thumbExtent.x, 0.F },
        scrollOrigin + scrollOffset + thumbExtent,
        scrollOrigin + scrollOffset + math::vec2 { 0.F, thumbExtent.y },
        _scrollThumbColor
    );

    /**
     *
     */
    cmd_->popScissor();
}

void ScrollContainer::shift(cref<UIContext> ctx_, cref<math::vec2> offset_) {

    const math::vec2 scrollOffset { _scrollValue * _overflowSize };
    ReflowContainer::shift(ctx_, offset_ - scrollOffset);

    /**
     *
     */
    this->_transform.offsetX = offset_.x;
    this->_transform.offsetY = offset_.y;
}

bool ScrollContainer::onScrollEvent(cref<math::ivec2> pointer_, cref<math::vec2> value_) {

    if (_nodes.empty()) {
        return ReflowContainer::onScrollEvent(pointer_, value_);
    }

    constexpr float scrollInvSpeed = 20.F;
    const auto diff { value_ * (_overflowSize / scrollInvSpeed) };
    const auto diffNorm { diff / _overflowSize };

    _scrollValue += diffNorm;
    _scrollValue.x = math::clamp(_scrollValue.x, 0.F, 1.F);
    _scrollValue.y = math::clamp(_scrollValue.y, 0.F, 1.F);

    return true;
}

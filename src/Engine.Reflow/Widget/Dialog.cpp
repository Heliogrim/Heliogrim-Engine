#include "Dialog.hpp"

#include "../Style/BoundStyleSheet.hpp"

using namespace ember::engine::reflow;
using namespace ember;

Dialog::Dialog(mref<sptr<BoundStyleSheet>> style_) :
    Popup(),
    _style(style_) {}

Dialog::~Dialog() = default;

string Dialog::getTag() const noexcept {
    return _STD format(R"(Dialog <{:#x}>)", reinterpret_cast<u64>(this));
}

void Dialog::repackChildren() {

    _children.clear();

    if (_titleBar) {
        _children.push_back(_titleBar);
    }

    if (_content) {
        _children.push_back(_content);
    }
}

sptr<Widget> Dialog::getTitleBar() const noexcept {
    return _titleBar;
}

sptr<Widget> Dialog::getContent() const noexcept {
    return _content;
}

void Dialog::setContent(cref<sptr<Widget>> content_) {

    content_->setParent(shared_from_this());

    if (_content && _content->parent().get() == this) {
        _content->setParent(nullptr);
    }

    _content = content_;
    repackChildren();
    markAsPending();
}

void Dialog::render(const ptr<ReflowCommandBuffer> cmd_) {

    const math::vec2 off { screenOffset() };
    const math::vec2 size { innerSize() };

    const math::vec2 c0 { off.x, off.y };
    const math::vec2 c1 { off.x + size.x, off.y };
    const math::vec2 c2 { off.x + size.x, off.y + size.y };
    const math::vec2 c3 { off.x, off.y + size.y };

    cmd_->drawQuad(c0, c1, c2, c3, _computedStyle.color.attr);

    /* TODO: Relocate */
    if (_content) {
        _content->render(cmd_);
    }
}

void Dialog::flow(
    cref<FlowContext> ctx_,
    cref<math::vec2> space_,
    cref<math::vec2> limit_,
    ref<StyleKeyStack> styleStack_
) {

    styleStack_.pushLayer();
    _computedStyle = _style->compute(shared_from_this(), styleStack_);

    const bool autoWidth { _computedStyle.width.attr.type == ReflowUnitType::eAuto };
    const bool autoHeight { _computedStyle.height.attr.type == ReflowUnitType::eAuto };

    math::vec2 local { 0.F };

    if (_computedStyle.width.attr.type == ReflowUnitType::eRelative) {
        local.x = _computedStyle.width.attr.value * space_.x;
    } else if (_computedStyle.width.attr.type == ReflowUnitType::eAbsolute) {
        local.x = _computedStyle.width.attr.value;
    }

    if (_computedStyle.height.attr.type == ReflowUnitType::eRelative) {
        local.y = _computedStyle.height.attr.value * space_.y;
    } else if (_computedStyle.height.attr.type == ReflowUnitType::eAbsolute) {
        local.y = _computedStyle.height.attr.value;
    }

    /**/

    if (_computedStyle.minWidth.attr.type == ReflowUnitType::eRelative) {
        local.x = MAX(local.x, _computedStyle.minWidth.attr.value * space_.x);
    } else if (_computedStyle.minWidth.attr.type == ReflowUnitType::eAbsolute) {
        local.x = MAX(local.x, _computedStyle.minWidth.attr.value);
    }

    if (_computedStyle.minHeight.attr.type == ReflowUnitType::eRelative) {
        local.y = MAX(local.y, _computedStyle.minHeight.attr.value * space_.y);
    } else if (_computedStyle.minHeight.attr.type == ReflowUnitType::eAbsolute) {
        local.y = MAX(local.y, _computedStyle.minHeight.attr.value);
    }

    /**/

    math::vec2 titleBound { 0.F };
    math::vec2 contentBound { 0.F };

    if (_titleBar) {
        _titleBar->flow(ctx_, local, limit_, styleStack_);
        titleBound = _titleBar->outerSize();
    }

    if (_content) {
        _content->flow(ctx_, local, limit_, styleStack_);
        contentBound = _content->outerSize();
    }

    /**/

    local.x = _STD max({ local.x, titleBound.x, contentBound.x });
    local.y = _STD max({ local.y, titleBound.y + contentBound.y });

    /**/

    if (_computedStyle.maxWidth.attr.type == ReflowUnitType::eRelative) {
        local.x = MIN(local.x, _computedStyle.maxWidth.attr.value * space_.x);
    } else if (_computedStyle.maxWidth.attr.type == ReflowUnitType::eAbsolute) {
        local.x = MIN(local.x, _computedStyle.maxWidth.attr.value);
    }

    if (_computedStyle.maxHeight.attr.type == ReflowUnitType::eRelative) {
        local.y = MIN(local.y, _computedStyle.maxHeight.attr.value * space_.y);
    } else if (_computedStyle.maxHeight.attr.type == ReflowUnitType::eAbsolute) {
        local.y = MIN(local.y, _computedStyle.maxHeight.attr.value);
    }

    /**/

    auto diff { math::vec2 { MAX(titleBound.x, contentBound.x), titleBound.y + contentBound.y } - local };

    /**
     * TODO: Overflow Behaviour - Flex Shrink
     */

    /**
     * TODO: Underflow Behaviour - Flex Grow
     */
    if (diff.x < 0.F || diff.y < 0.F) {

        if (_titleBar) {
            _titleBar->flow(ctx_, local, math::vec2 { local.x, local.y - contentBound.y }, styleStack_);
        }

        if (_content) {
            _content->flow(ctx_, local, math::vec2 { local.x, local.y - titleBound.y }, styleStack_);
        }
    }

    /**/

    _compositeSize = local;
    styleStack_.popLayer();
}

void Dialog::shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) {

    math::vec2 titleSize { 0.F };
    if (_titleBar) {
        titleSize = _titleBar->outerSize();
    }

    /**/

    if (_titleBar) {
        _titleBar->shift(ctx_, offset_);
    }

    if (_content) {
        _content->shift(ctx_, math::vec2 { offset_.x, offset_.y + titleSize.y });
    }

}

const ptr<const Children> Dialog::children() const {
    return &_children;
}

math::vec2 Dialog::outerSize() const noexcept {
    return _compositeSize;
}

math::vec2 Dialog::innerSize() const noexcept {
    return _compositeSize;
}

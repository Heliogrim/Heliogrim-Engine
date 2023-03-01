#include "Box.hpp"

#include "../Style/BoundStyleSheet.hpp"
#include "../Layout/Constraints.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Box::Box(mref<sptr<BoundStyleSheet>> style_) :
    Widget(),
    _style(_STD move(style_)) {}

Box::~Box() = default;

cref<sptr<BoundStyleSheet>> Box::style() const noexcept {
    return _style;
}

ref<sptr<BoundStyleSheet>> Box::style() noexcept {
    return _style;
}

const ptr<const Children> Box::children() const {
    return &_children;
}

void Box::addChild(cref<sptr<Widget>> child_) {
    child_->setParent(shared_from_this());
    _children.push_back(child_);
    markAsPending();
}

void Box::removeChild(cref<sptr<Widget>> child_) {
    _children.push_back(child_);

    if (child_->parent().get() == this) {
        child_->setParent(nullptr);
    }

    markAsPending();
}

void Box::clearChildren() {

    for (const auto& child : _children) {
        if (child->parent().get() == this) {
            child->setParent(nullptr);
        }
    }

    _children.clear();
    markAsPending();
}

void Box::render(const ptr<ReflowCommandBuffer> cmd_) {

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

float Box::shrinkFactor() const noexcept {
    return _computedStyle.reflowShrink.attr;
}

float Box::growFactor() const noexcept {
    return _computedStyle.reflowGrow.attr;
}

math::vec2 Box::outerSize() const noexcept {

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

math::vec2 Box::innerSize() const noexcept {
    return _innerSize;
}

math::vec2 Box::screenOffset() const noexcept {
    return _screenOff;
}

bool Box::willChangeLayout(cref<math::vec2> space_, cref<StyleKeyStack> styleStack_) const noexcept {

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

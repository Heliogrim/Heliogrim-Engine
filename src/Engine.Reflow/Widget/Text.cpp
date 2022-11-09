#include "Text.hpp"

#include "../Style/BoundStyleSheet.hpp"
#include "../Layout/Constraints.hpp"

using namespace ember::engine::reflow;
using namespace ember;

Text::Text(mref<sptr<BoundStyleSheet>> style_) :
    Widget(),
    _style(_STD move(style_)),
    _text(),
    _contentSize(),
    _screenOff() {}

Text::~Text() = default;

cref<string> Text::getText() const noexcept {
    return _text;
}

void Text::setText(cref<string> text_) {
    _text = text_;
    markAsPending();
}

math::vec2 Text::contentSize(cref<math::vec2> space_) const {

    const auto fss { (_computedStyle.font.attr)->nextFontSize(static_cast<u32>(_computedStyle.fontSize.attr)) };
    const auto fontScale { _computedStyle.fontSize.attr / static_cast<float>(fss) };

    float lineBound { 0.F };
    if (_computedStyle.maxWidth->type == ReflowUnitType::eRelative) {
        lineBound = _computedStyle.maxWidth->value * space_.x;
    } else if (_computedStyle.maxWidth->type == ReflowUnitType::eAbsolute) {
        lineBound = _computedStyle.maxWidth->value;
    }

    float fwd { 0.F };
    u32 lines { 0ui32 };
    math::vec2 size { 0.F };

    for (const auto& letter : _text) {

        const auto& glyph { (_computedStyle.font.attr)->glyph(static_cast<u32>(letter), fss) };
        fwd += glyph->_advance * fontScale;

        if (_computedStyle.maxWidth->type != ReflowUnitType::eAuto && fwd > lineBound) {
            fwd = glyph->_advance * fontScale;
            ++lines;
        }

        size.x = MAX(size.x, fwd);
    }

    /**/

    if (_computedStyle.wrap.attr == ReflowWrap::eNoWrap) {
        lines = 1ui32;
    } else if (_computedStyle.textEllipse.attr) {
        lines = MIN(lines, _computedStyle.textEllipse.attr);
    }

    size.y = static_cast<float>(lines) * _computedStyle.fontSize.attr * _computedStyle.lineHeight.attr;
    return size;
}

void Text::render(const ptr<ReflowCommandBuffer> cmd_) {

    math::vec2 off { _screenOff };
    off.x += _computedStyle.margin->x;
    off.y += _computedStyle.margin->y;

    /**/

    const math::fExtent2D scissor { _innerSize.x, _innerSize.y, off.x, off.y };
    cmd_->pushIntersectScissor(scissor);

    /**/

    const u8 align { static_cast<u8>(_computedStyle.textAlign.attr) };
    if ((align & 0b0000'1110ui8) == 0x0ui8) {

        /* Align Left */
        // off.x += _computedStyle.margin.x;

    } else if ((align & 0b0000'1111ui8) == 0x2ui8) {

        /* Align Center */
        const math::vec2 diff { _innerSize.x - _contentSize.x, _innerSize.y - _contentSize.y };
        off.x += diff.x * 0.5F;

    } else if ((align & 0b0000'1111ui8) == 0x4ui8) {

        /* Align Right */
        const math::vec2 diff { _innerSize.x - _contentSize.x, _innerSize.y - _contentSize.y };
        off.x += diff.x;
    }

    if ((align & 0b1110'0000ui8) == 0x0ui8) {

        /* Vertical Align Top */
        // off.y += _computedStyle.margin.y;

    } else if ((align & 0b1111'0000ui8) == 0x20ui8) {

        /* Vertical Align Middle */
        const math::vec2 diff { _innerSize.x - _contentSize.x, _innerSize.y - _contentSize.y };
        off.y += diff.y * 0.5F;

    } else if ((align & 0b1111'0000ui8) == 0x40ui8) {

        /* Vertical Align Bottom */
        const math::vec2 diff { _innerSize.x - _contentSize.x, _innerSize.y - _contentSize.y };
        off.y += diff.y;
    }

    /**/

    if (not _computedStyle.textEllipse.attr) {
        cmd_->drawText(off, _text, *_computedStyle.font.attr, _computedStyle.fontSize.attr, _computedStyle.color.attr);

    } else {

        const auto fss { (_computedStyle.font.attr)->nextFontSize(static_cast<u32>(_computedStyle.fontSize.attr)) };
        const auto fontScale { _computedStyle.fontSize.attr / static_cast<float>(fss) };
        const auto lineBound { _contentSize.x };

        float fccw { 0.F };
        u32 wraps { 0ui32 };
        u32 j { 0ui32 };

        for (u32 i { 0ui32 }; i < _text.length(); ++i) {

            const auto& letter { _text[i] };
            const auto& glyph { (_computedStyle.font.attr)->glyph(static_cast<u32>(letter), fss) };

            fccw += glyph->_advance * fontScale;

            if (_computedStyle.maxWidth->type != ReflowUnitType::eAuto && fccw > lineBound && wraps < _computedStyle.
                textEllipse.attr) {

                cmd_->drawText(off, string_view { _text.begin() + j, _text.begin() + i }, *_computedStyle.font.attr,
                    _computedStyle.fontSize.attr,
                    _computedStyle.color.attr);

                off.y += _computedStyle.fontSize.attr * _computedStyle.lineHeight.attr;

                fccw = glyph->_advance * fontScale;
                j = i;
                ++wraps;
            }

        }

        if (fccw <= lineBound && wraps < _computedStyle.textEllipse.attr) {
            cmd_->drawText(off, string_view { _text.begin() + j, _text.end() }, *_computedStyle.font.attr,
                _computedStyle.fontSize.attr,
                _computedStyle.color.attr);
        }

    }

    /**/

    cmd_->popScissor();
}

void Text::flow(cref<FlowContext> ctx_, cref<math::vec2> space_, ref<StyleKeyStack> styleStack_) {

    styleStack_.pushLayer();
    _computedStyle = _style->compute(shared_from_this(), styleStack_);

    const bool autoWidth { _computedStyle.width->type == ReflowUnitType::eAuto };
    const bool autoHeight { _computedStyle.height->type == ReflowUnitType::eAuto };

    math::vec2 local { 0.F };

    if (_computedStyle.width->type == ReflowUnitType::eRelative) {
        local.x = _computedStyle.width->value * space_.x;
    } else if (_computedStyle.width->type == ReflowUnitType::eAbsolute) {
        local.x = _computedStyle.width->value;
    }

    if (_computedStyle.height->type == ReflowUnitType::eRelative) {
        local.y = _computedStyle.height->value * space_.y;
    } else if (_computedStyle.height->type == ReflowUnitType::eAbsolute) {
        local.y = _computedStyle.height->value;
    }

    /**/

    if (_computedStyle.minWidth->type == ReflowUnitType::eRelative) {
        local.x = MAX(local.x, _computedStyle.minWidth->value * space_.x);
    } else if (_computedStyle.minWidth->type == ReflowUnitType::eAbsolute) {
        local.x = MAX(local.x, _computedStyle.minWidth->value);
    }

    if (_computedStyle.minHeight->type == ReflowUnitType::eRelative) {
        local.y = MAX(local.y, _computedStyle.minHeight->value * space_.y);
    } else if (_computedStyle.minHeight->type == ReflowUnitType::eAbsolute) {
        local.y = MAX(local.y, _computedStyle.minHeight->value);
    }

    /**/

    const auto baseSize { contentSize(space_) };
    _contentSize = baseSize;
    _innerSize = math::compMax<float>(baseSize, local);

    styleStack_.popLayer();
    clearPending();

    _prevSpace = space_;
    _prevStyle.clear();
    styleStack_.compress(_prevStyle);
}

void Text::shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) {
    _screenOff = offset_;
}

math::vec2 Text::outerSize() const noexcept {

    auto size { innerSize() };

    if (not _computedStyle.margin->zero()) {
        size.x += _computedStyle.margin->x;
        size.x += _computedStyle.margin->z;
        size.y += _computedStyle.margin->y;
        size.y += _computedStyle.margin->w;
    }

    return size;
}

math::vec2 Text::innerSize() const noexcept {
    return _innerSize;
}

math::vec2 Text::screenOffset() const noexcept {
    return _screenOff;
}

bool Text::willChangeLayout(cref<math::vec2> space_, cref<StyleKeyStack> styleStack_) const noexcept {

    if (_state.pending) {
        return true;
    }

    if (layout::hasStyleChanged(_prevStyle, styleStack_)) {
        return true;
    }

    if (_prevSpace != space_) {
        return not layout::hasConstSize(_computedStyle);
    }

    return Widget::willChangeLayout(space_, styleStack_);
}

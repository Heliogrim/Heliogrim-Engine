#include "InputText.hpp"

#include "Editor.UI/Color/Dark.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

InputText::InputText() :
    Input(),
    _placeholder(),
    _font(nullptr),
    _fontSize(16.F),
    _lineHeight(1.F),
    _ellipseLines(0ui32),
    _textAlign(0b0001'0001ui8),
    _baseColor(editor::ui::color::Dark::white),
    _baseBackground(editor::ui::color::Dark::backgroundInnerField),
    _raisedColor(editor::ui::color::Dark::white),
    _raisedBackground(editor::ui::color::Dark::backgroundInnerFieldDarken),
    _focusColor(editor::ui::color::Dark::white),
    _focusBackground(editor::ui::color::Dark::backgroundInnerFieldDarken) {}

void InputText::render(const ptr<UICommandBuffer> cmd_) {

    const auto& tf { _transform };

    const math::fExtent2D scissor { tf };
    cmd_->pushIntersectScissor(scissor);

    /**/

    Reflow::renderContainer(this, cmd_);

    /**/

    assert(_font);
    math::vec2 c0 { tf.offsetX, tf.offsetY };

    /**/
    if ((_textAlign & 0b0000'1110ui8) == 0x0ui8) {

        // Align Left
        c0.x += _padding.x;

    } else if ((_textAlign & 0b0000'1111ui8) == 0x2ui8) {

        // Align Center
        const math::vec2 diff { tf.width - _contentSize.x, tf.height - _contentSize.y };
        c0.x += diff.x * 0.5F;

    } else if ((_textAlign & 0b0000'1111ui8) == 0x4ui8) {

        // Align Right
        const math::vec2 diff { tf.width - _contentSize.x, tf.height - _contentSize.y };
        c0.x += diff.x;
    }

    if ((_textAlign & 0b1110'0000ui8) == 0x0ui8) {

        // Vertical Align Top
        c0.y += _padding.y;

    } else if ((_textAlign & 0b1111'0000ui8) == 0x20ui8) {

        // Vertical Align Middle
        const math::vec2 diff { tf.width - _contentSize.x, tf.height - _contentSize.y };
        c0.y += diff.y * 0.5F;

    } else if ((_textAlign & 0b1111'0000ui8) == 0x40ui8) {

        // Vertical Align Bottom
        const math::vec2 diff { tf.width - _contentSize.x, tf.height - _contentSize.y };
        c0.y += diff.y;

    }

    /**/

    color cl { _baseColor };

    if (hovered()) {
        cl = _raisedColor;
    }

    if (focused()) {
        cl = _focusColor;
    }

    cref<string> text { _value.empty() && !focused() ? _placeholder : _value };
    if (_ellipseLines > 0ui32) {

        const auto fss { _font->nextFontSize(static_cast<u32>(_fontSize)) };
        const math::vec2 fontScale { _fontSize / static_cast<float>(fss) };
        const auto hasBounds { _maxExtent.x > 0.F };

        float fccw { 0.F };
        u32 wraps { 0ui32 };
        u32 j { 0ui32 };

        for (u32 i { 0ui32 }; i < text.length(); ++i) {

            const auto& letter { text[i] };
            const auto& glyph { _font->glyph(static_cast<u32>(letter), fss) };

            fccw += glyph->_advance * fontScale.x;

            if (hasBounds && fccw > _maxExtent.x && wraps < _ellipseLines) {
                cmd_->drawText(c0, string_view { text.begin() + j, text.begin() + i }, *_font, _fontSize, cl);
                c0.y += _fontSize * _lineHeight;

                fccw = glyph->_advance * fontScale.x;
                j = i;
                ++wraps;
            }
        }

        if (fccw <= _maxExtent.x && wraps < _ellipseLines) {
            cmd_->drawText(c0, string_view { text.begin() + j, text.end() }, *_font, _fontSize, cl);
        }

    } else {
        cmd_->drawText(c0, text, *_font, _fontSize, cl);
    }

    /**/
    cmd_->popScissor();
}

engine::color InputText::statedColor() const noexcept {
    if (focused()) {
        return _focusBackground;
    }
    if (hovered()) {
        return _raisedBackground;
    }
    return _baseBackground;
}

math::vec2 InputText::baseFlowSize() const {

    math::vec2 dim { 0.F };

    const auto fss { _font->nextFontSize(static_cast<u32>(_fontSize)) };
    const math::vec2 fontScale { _fontSize / static_cast<float>(fss) };

    float fwd { 0.F };
    u32 lines { 1ui32 };

    const auto& text { _value.empty() && not focused() ? _placeholder : _value };
    for (const auto& letter : text) {

        const auto& glyph { _font->glyph(static_cast<u32>(letter), fss) };
        fwd += glyph->_advance * fontScale.x;

        if (_maxExtent.x > 0.F && fwd > _maxExtent.x) {
            fwd = glyph->_advance * fontScale.x;
            ++lines;
        }

        dim.x = MAX(dim.x, fwd);
    }

    /**/
    if (_reflowOverflow == ReflowOverflow::eDiscard || _reflowOverflow == ReflowOverflow::eNone) {
        lines = 1ui32;

    } else if (_reflowOverflow == ReflowOverflow::eAuto && _reflowWrapping == ReflowWrapping::eWrap) {
        if (_ellipseLines) {
            lines = MIN(lines, _ellipseLines);
        }
    }

    /**/
    dim.y = static_cast<float>(lines) * _fontSize * _lineHeight;
    return dim;
}

math::vec2 InputText::flow(cref<UIContext> ctx_, cref<math::vec2> outerSpace_) {

    const bool autoWidth { _extent.x < 0.F };
    const bool autoHeight { _extent.y < 0.F };

    auto local = _extent * outerSpace_;

    if (autoWidth) {
        local.x = 0.F;
    }
    if (autoHeight) {
        local.y = 0.F;
    }

    /**/
    local = math::compMax<float>(local, _minExtent);

    /**/
    if (not _padding.zero()) {
        local.x = MAX(local.x - (_padding.x + _padding.z), 0.0F);
        local.y = MAX(local.y - (_padding.y + _padding.w), 0.0F);
    }

    /**/
    const auto baseSize { baseFlowSize() };

    _contentSize = baseSize;
    local = math::compMax<float>(local, baseSize);

    /**/
    if (not _padding.zero()) {
        local.x += (_padding.x + _padding.z);
        local.y += (_padding.y + _padding.w);
    }

    /**/
    _transform.width = local.x;
    _transform.height = local.y;

    /**/
    local.x += (_margin.x + _margin.z);
    local.y += (_margin.y + _margin.w);

    return local;
}

cref<string> InputText::placeholder() const noexcept {
    return _placeholder;
}

void InputText::setPlaceholder(cref<string> text_) {
    _placeholder = text_;
}

bool InputText::onMouseButtonEvent(cref<math::ivec2> pointer_, u32 button_, bool down_, u32 modifier_) {

    if (!focused()) {
        requestFocus();
    }

    return Input<std::string>::onMouseButtonEvent(pointer_, button_, down_, modifier_);
}

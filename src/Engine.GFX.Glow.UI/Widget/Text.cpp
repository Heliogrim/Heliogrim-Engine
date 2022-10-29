#include "Text.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

Text::Text() :
    Widget(),
    _text(""sv),
    _font(nullptr),
    _fontSize(16.F),
    _lineHeight(1.F),
    _ellipseLines(0ui32) {

    _reflowOverflow = ReflowOverflow::eAuto;
    _reflowWrapping = ReflowWrapping::eWrap;

    _extent.x = 0.F;
    _extent.y = 0.F;
    _minExtent.x = -1.F;
    _minExtent.y = -1.F;

    _color = color { 186.F, 186.F, 186.F, 255.F };
    _raisedColor = color { 255.F, 255.F, 255.F, 255.F };

    _textAlign = 0b0001'0001;
}

Text::~Text() {}

void Text::render(const ptr<UICommandBuffer> cmd_) {

    // Warning: Temporary color patch
    const color cl { hovered() ? _raisedColor : _color };
    const auto& tf { _transform };

    const math::fExtent2D scissor { tf };
    cmd_->pushIntersectScissor(scissor);

    #if TRUE
    assert(_font);
    math::vec2 c0 { tf.offsetX, tf.offsetY };

    /**/
    if ((_textAlign & 0b0000'1111ui8) == 0x2ui8) {

        // Align Center
        const math::vec2 diff { tf.width - _contentSize.x, tf.height - _contentSize.y };
        c0.x += diff.x * 0.5F;

    } else if ((_textAlign & 0b0000'1111ui8) == 0x4ui8) {

        // Align Right
        const math::vec2 diff { tf.width - _contentSize.x, tf.height - _contentSize.y };
        c0.x += diff.x;
    }

    if ((_textAlign & 0b1111'0000ui8) == 0x20ui8) {

        // Vertical Align Middle
        const math::vec2 diff { tf.width - _contentSize.x, tf.height - _contentSize.y };
        c0.y += diff.y * 0.5F;

    } else if ((_textAlign & 0b1111'0000ui8) == 0x40ui8) {

        // Vertical Align Bottom
        const math::vec2 diff { tf.width - _contentSize.x, tf.height - _contentSize.y };
        c0.y += diff.y;

    }

    /**/

    if (_ellipseLines > 0ui32) {

        const auto fss { _font->nextFontSize(static_cast<u32>(_fontSize)) };
        const math::vec2 fontScale { _fontSize / static_cast<float>(fss) };
        const auto hasBounds { _maxExtent.x > 0.F };

        float fccw { 0.F };
        u32 wraps { 0ui32 };
        u32 j { 0ui32 };

        for (u32 i { 0ui32 }; i < _text.length(); ++i) {

            const auto& letter { _text[i] };
            const auto& glyph { _font->glyph(static_cast<u32>(letter), fss) };

            fccw += glyph->_advance * fontScale.x;

            if (hasBounds && fccw > _maxExtent.x && wraps < _ellipseLines) {
                cmd_->drawText(c0, string_view { _text.begin() + j, _text.begin() + i }, *_font, _fontSize, cl);
                c0.y += _fontSize * _lineHeight;

                fccw = glyph->_advance * fontScale.x;
                j = i;
                ++wraps;
            }
        }

        if (fccw <= _maxExtent.x && wraps < _ellipseLines) {
            cmd_->drawText(c0, string_view { _text.begin() + j, _text.end() }, *_font, _fontSize, cl);
        }

    } else {
        cmd_->drawText(c0, _text, *_font, _fontSize, cl);
    }
    #else

    const math::vec2 c0 { tf.offsetX, tf.offsetY };
    const math::vec2 c1 { tf.offsetX + tf.width, tf.offsetY };
    const math::vec2 c2 { tf.offsetX + tf.width, tf.offsetY + tf.height };
    const math::vec2 c3 { tf.offsetX, tf.offsetY + tf.height };

    cmd_->drawQuad(c0, c1, c2, c3, cl);
    #endif

    /**/
    cmd_->popScissor();
}

math::vec2 Text::baseFlowSize() const {

    math::vec2 dim { 0.F };

    const auto fss { _font->nextFontSize(static_cast<u32>(_fontSize)) };
    const math::vec2 fontScale { _fontSize / static_cast<float>(fss) };

    float fwd { 0.F };
    u32 lines { 1ui32 };

    for (const auto& letter : _text) {

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

math::vec2 Text::flow(cref<UIContext> ctx_, cref<math::vec2> outerSpace_) {

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

    #if FALSE
    /**/

    const math::vec2 fontScale { _fontSize / _font->fontSize() };
    const auto& bounds { _maxExtent };

    /**/
    float accw { 0.F };
    u32 wraps { 0ui32 };
    for (const auto& letter : _text) {
        const auto& glyph { _font->glyph(static_cast<u32>(letter), static_cast<u32>(_fontSize)) };
        accw += glyph->_advance * fontScale.x;

        if (bounds.x > 0.F && accw > bounds.x) {
            accw = glyph->_advance * fontScale.x;
            ++wraps;
        }
    }

    /**/
    math::vec2 report { accw, _fontSize * _lineHeight };

    if (wraps > 0ui32 && _ellipseLines > 0ui32) {
        report.x = bounds.x;
        report.y = _fontSize * _lineHeight * static_cast<float>(MIN(wraps + 1ui32, _ellipseLines));
    }

    if (_maxExtent.x > 0.F) {
        report.x = MIN(report.x, _maxExtent.x);
    }

    /**/
    _transform.width = report.x;
    _transform.height = report.y;

    /**/
    report.x += (_margin.x + _margin.z);
    report.y += (_margin.y + _margin.w);

    return report;
    #endif
}

math::vec2 Text::reflow(cref<math::vec2> available_, cref<math::vec2> reflow_) {
    return math::vec2 { 0.10546875F, 0.02222222F };
}

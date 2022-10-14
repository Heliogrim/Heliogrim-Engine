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

    _extent.x = 0.F;
    _extent.y = 0.F;
    _minExtent.x = 120.F;
    _minExtent.y = 20.F;
}

Text::~Text() {}

void Text::render(const ptr<UICommandBuffer> cmd_) {

    color cl { 255.F, 255.F, 255.F, 255.F };
    if (hovered()) {
        cl.g = 127.F;
        cl.b = 127.F;
    }

    const auto& tf { _transform };

    #if TRUE
    assert(_font);
    math::vec2 c0 { tf.offsetX, tf.offsetY };

    if (_ellipseLines > 0ui32) {

        const math::vec2 fontScale { _fontSize / _font->fontSize() };
        const auto hasBounds { _maxExtent.x > 0.F };

        float fccw { 0.F };
        u32 wraps { 0ui32 };
        u32 j { 0ui32 };

        for (u32 i { 0ui32 }; i < _text.length(); ++i) {

            const auto& letter { _text[i] };
            const auto& glyph { _font->glyph(static_cast<u32>(letter)) };

            fccw += glyph.advance * fontScale.x;

            if (hasBounds && fccw > _maxExtent.x && wraps < _ellipseLines) {
                cmd_->drawText(c0, string_view { _text.begin() + j, _text.begin() + i }, *_font, _fontSize, cl);
                c0.y += _fontSize * _lineHeight;

                fccw = glyph.advance * fontScale.x;
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
}

math::vec2 Text::flow(cref<UIContext> ctx_, cref<math::vec2> available_) {

    const math::vec2 fontScale { _fontSize / _font->fontSize() };
    const auto& bounds { _maxExtent };

    /**/
    float accw { 0.F };
    u32 wraps { 0ui32 };
    for (const auto& letter : _text) {
        const auto& glyph { _font->glyph(static_cast<u32>(letter)) };
        accw += glyph.advance * fontScale.x;

        if (bounds.x > 0.F && accw > bounds.x) {
            accw = glyph.advance * fontScale.x;
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
}

math::vec2 Text::reflow(cref<math::vec2> available_, cref<math::vec2> reflow_) {
    return math::vec2 { 0.10546875F, 0.02222222F };
}

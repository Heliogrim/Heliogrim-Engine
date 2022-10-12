#include "Font.hpp"

#include "Engine.GFX/Command/CommandPool.hpp"
#include "Engine.GFX/Command/CommandQueue.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

Font::Font():
    _fontSize(16),
    _fallback(),
    _first(0),
    _glyphCount(0) {}

Font::~Font() noexcept = default;

cref<string> Font::name() const noexcept {
    return _name;
}

bool Font::isBitmap() const noexcept {
    return true;
}

u32 Font::width() const noexcept {
    return _extent.x;
}

u32 Font::height() const noexcept {
    return _extent.y;
}

float Font::fontSize() const noexcept {
    return _fontSize;
}

cref<FontGlyph> Font::glyph(const u32 letter_) const noexcept {

    if (letter_ < _first || (_first + _glyphCount) <= letter_) {
        return _fallback;
    }

    return _glyphs[letter_ - _first];
}

cref<sptr<engine::gfx::Texture>> Font::atlas() const noexcept {
    return _atlas;
}

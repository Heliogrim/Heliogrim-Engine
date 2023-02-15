#include "Font.hpp"

#include <Engine.GFX/Texture/Texture.hpp>
#include "Engine.GFX/Command/CommandPool.hpp"
#include "Engine.GFX/Command/CommandQueue.hpp"

using namespace ember::engine::reflow;
using namespace ember;

Font::Font():
    _fontSize(16),
    _glyphCount(0),
    _ftFace(nullptr) {}

Font::~Font() noexcept {

    if (not _glyphs.empty()) {
        for (auto&& glyph : _glyphs) {
            delete glyph.second;
        }
        _glyphs.clear();
    }

};

cref<string> Font::name() const noexcept {
    return _name;
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

u32 Font::nextFontSize(const u32 size_) const noexcept {
    if (_sizes.empty()) {
        return 0ui32;
    }

    u32 nextSize { _sizes.front() };
    u32 sizeDiff {
        nextSize > size_ ?
            nextSize - size_ :
            size_ - nextSize
    };

    for (const auto& size : _sizes) {
        const u32 diff { size > size_ ? size - size_ : size_ - size };
        if (diff < sizeDiff) {
            nextSize = size;
            sizeDiff = diff;
        }
    }

    return nextSize;
}

u32 Font::encodeKey(const GlyphCode code_, const u32 fontSize_) const noexcept {
    return (code_ & 0x00FF'FFFF) | ((fontSize_ << 24) & 0xFF00'0000);
}

const ptr<const FontGlyph> Font::glyph(const GlyphCode code_, const u32 fontSize_) const noexcept {

    const auto nextSize { nextFontSize(fontSize_) };
    const auto key { encodeKey(code_, nextSize) };

    /**/

    const auto it { _glyphs.find(key) };
    if (it != _glyphs.end()) {
        return it->second;
    }

    return nullptr;
}

cref<sptr<::ember::engine::gfx::Texture>> Font::atlas() const noexcept {
    return _atlas;
}

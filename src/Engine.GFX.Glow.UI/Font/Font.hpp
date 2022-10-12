#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.GFX/Texture/Texture.hpp>

#include "FontGlyph.hpp"

namespace ember::engine::gfx::glow::ui {

    class Font {
    public:
        using this_type = Font;

    public:
        Font();

        ~Font() noexcept;

    //private:
    public:
        string _name;

    public:
        [[nodiscard]] cref<string> name() const noexcept;

    public:
        [[nodiscard]] bool isBitmap() const noexcept;

    //private:
    public:
        math::uivec2 _extent;

    public:
        [[nodiscard]] u32 width() const noexcept;

        [[nodiscard]] u32 height() const noexcept;

    //private:
    public:
        float _fontSize;

    public:
        [[nodiscard]] float fontSize() const noexcept;


    //private:
    public:
        Vector<FontGlyph> _glyphs;
        FontGlyph _fallback;

        u32 _first;
        u32 _glyphCount;

    public:
        [[nodiscard]] cref<FontGlyph> glyph(const u32 letter_) const noexcept;

    //private:
    public:
        sptr<Texture> _atlas;

    public:
        [[nodiscard]] cref<sptr<Texture>> atlas() const noexcept;
    };
}

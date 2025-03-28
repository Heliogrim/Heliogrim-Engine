#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.GFX/Texture/__fwd.hpp>

#include "FontGlyph.hpp"
#include "GlyphCode.hpp"

namespace hg::engine::reflow {
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

        //private:
    public:
        math::uivec2 _extent;

    public:
        [[nodiscard]] u32 width() const noexcept;

        [[nodiscard]] u32 height() const noexcept;

        //private:
    public:
        u32 _fontSize;
        Vector<u32> _sizes;

    public:
        [[nodiscard]] float fontSize() const noexcept;

        [[nodiscard]] u32 nextFontSize(const u32 size_) const noexcept;

        //private:
    public:
        DenseMap<u32, ptr<FontGlyph>> _glyphs;
        u32 _glyphCount;

        //private:
    public:
        [[nodiscard]] u32 encodeKey(const GlyphCode code_, const u32 fontSize_) const noexcept;

    public:
        [[nodiscard]] const ptr<const FontGlyph> glyph(const GlyphCode code_, const u32 fontSize_) const noexcept;

        //private:
    public:
        sptr<::hg::engine::gfx::Texture> _atlas;

    public:
        [[nodiscard]] cref<sptr<::hg::engine::gfx::Texture>> atlas() const noexcept;

        //private:
    public:
        ptr<void> _ftFace;
    };
}

#pragma once

#include <Engine.Common/Url.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.GFX.Glow.UI/Font/GlyphRanges.hpp>
#include <Engine.Resource/Loader/Loader.hpp>

#include "Data.hpp"
#include "FontLayout.hpp"

namespace ember::engine::assets {

    class Font :
        public Data<FontLayout> {
    public:
        inline static const asset_type_id typeId { "Font"_typeId };

    protected:
        Font(cref<asset_guid> guid_);

    public:
        Font(cref<asset_guid> guid_, mref<Vector<Url>> sources_);

    private:
        Vector<Url> _sources;

    public:
        [[nodiscard]] cref<Vector<Url>> sources() const noexcept;
    };

}

namespace ember::engine::res {
    template <>
    struct LoaderOptions<::ember::engine::assets::Font> {
        Vector<gfx::glow::ui::GlyphRange> ranges;
        Vector<gfx::glow::ui::GlyphCode> glyphs;

        /**/

        Vector<u32> fontSizes;
    };
}

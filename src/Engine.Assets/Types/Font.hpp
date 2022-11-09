#pragma once

#include <Engine.Common/Url.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Reflow/Font/GlyphRanges.hpp>
#include <Engine.Resource/Loader/Loader.hpp>

#include "Asset.hpp"

namespace ember::engine::assets {

    class Font :
        public Asset {
    public:
        template <typename>
        friend class serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "Font"_typeId };

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
        Vector<reflow::GlyphRange> ranges;
        Vector<reflow::GlyphCode> glyphs;

        /**/

        Vector<u32> fontSizes;
    };
}

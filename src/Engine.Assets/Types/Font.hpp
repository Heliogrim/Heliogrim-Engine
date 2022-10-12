#pragma once

#include <Engine.Common/Url.hpp>
#include <Engine.Common/Collection/Vector.hpp>

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

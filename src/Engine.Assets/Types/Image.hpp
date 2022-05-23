#pragma once

#include <Engine.Common/Url.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "Data.hpp"
#include "ImageLayout.hpp"

namespace ember::engine::assets {
    class Image :
        public Data<ImageLayout> {
    public:
        inline static const asset_type_id typeId { "Image"_typeId };

    protected:
        Image(cref<asset_guid> guid_);

    public:
        Image(cref<asset_guid> guid_, mref<Vector<Url>> sources_);

    private:
        Vector<Url> _sources;
    };
}

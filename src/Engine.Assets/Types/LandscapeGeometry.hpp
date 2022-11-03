#pragma once

#include <Engine.Common/Url.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "Geometry.hpp"

namespace ember::engine::assets {

    class LandscapeGeometry :
        public Geometry {
    public:
        template <typename>
        friend class serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "LandscapeGeometry"_typeId };

    protected:
        LandscapeGeometry(cref<asset_guid> guid_);

    public:
        LandscapeGeometry(cref<asset_guid> guid_, mref<Vector<Url>> sources_);

    private:
        Vector<Url> _sources;
    };
}

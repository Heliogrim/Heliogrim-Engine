#pragma once

#include <Engine.Common/Url.hpp>
#include <Engine.Common/Math/Bounding.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "Geometry.hpp"
#include "StaticGeometryLayout.hpp"

namespace ember::engine::assets {

    class StaticGeometry :
        public Geometry<StaticGeometryLayout> {
    public:
        inline static constexpr asset_type_id typeId { "StaticGeometry"_typeId };

    protected:
        StaticGeometry(cref<asset_guid> guid_);

    public:
        StaticGeometry(cref<asset_guid> guid_, mref<Vector<Url>> sources_);

    private:
        Vector<Url> _sources;

    public:
        [[nodiscard, deprecated]] u32 getMaterialCount();

        [[nodiscard, deprecated]] u64 getVertexCount();

        [[nodiscard, deprecated]] u64 getIndexCount();

        [[nodiscard, deprecated]] u64 getFaceCount();

        [[nodiscard, deprecated]] bool hasUvCoords();

        [[nodiscard, deprecated]] bool hasUvm();

        [[nodiscard, deprecated]] bool hasLods();

        [[nodiscard, deprecated]] bool isStreamable();

        [[nodiscard]] math::Bounding getBoundary();
    };
}

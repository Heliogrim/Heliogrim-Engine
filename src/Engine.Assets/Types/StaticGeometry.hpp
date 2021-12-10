#pragma once

#include <Engine.Common/Math/Bounding.hpp>

#include "Geometry.hpp"
#include "StaticGeometryLayout.hpp"

namespace ember::engine::assets {

    class StaticGeometry :
        public Geometry<StaticGeometryLayout> {
    public:
        inline static const asset_type_id type_id { "StaticGeometry"_typeId };

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

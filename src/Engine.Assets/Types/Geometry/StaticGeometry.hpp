#pragma once

#include <Engine.Common/Url.hpp>
#include <Engine.Common/Math/Bounding.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "../Geometry.hpp"
#include "StaticGeometryLayout.hpp"

namespace ember::engine::assets {

    class StaticGeometry :
        public Geometry<StaticGeometryLayout> {
    public:
        inline static constexpr asset_type_id typeId { "StaticGeometry"_typeId };

    protected:
        StaticGeometry(cref<asset_guid> guid_);

    public:
        StaticGeometry(
            cref<asset_guid> guid_,
            mref<Vector<Url>> sources_,
            cref<u64> vertexCount_,
            cref<u64> indexCount_
        );

    private:
        Vector<Url> _sources;

    public:
        [[nodiscard]] cref<Vector<Url>> sources() const noexcept;

        //private:
    public:
        u64 _vertexCount;
        u64 _indexCount;

        u64 _clusterCount;
        u64 _clusterDepth;

    public:
        [[nodiscard]] u64 getVertexCount() const;

        [[nodiscard]] u64 getIndexCount() const;

        [[nodiscard, deprecated]] u64 getFaceCount() const;

        [[nodiscard, deprecated]] u32 getMaterialCount() const;

        [[nodiscard, deprecated]] bool hasUvCoords();

        [[nodiscard, deprecated]] bool hasUvm();

        [[nodiscard, deprecated]] bool hasLods();

        [[nodiscard, deprecated]] bool isStreamable();

        [[nodiscard]] math::Bounding getBoundary();
    };
}

#pragma once

#include <Engine.Graphics/Geometry/PolygonMesh.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

namespace hg::engine::graphics {
    typedef resource::Resource<PolygonMesh> StaticGeometryResource;
}

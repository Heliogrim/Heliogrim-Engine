#pragma once

#include <Engine.GFX/Geometry/PolygonMesh.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

namespace hg::engine::gfx {
    typedef resource::Resource<PolygonMesh> StaticGeometryResource;
}

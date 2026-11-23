#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Graphics;
import Engine.Resource;
#endif

namespace hg::engine::graphics {
    typedef resource::Resource<PolygonMesh> StaticGeometryResource;
}

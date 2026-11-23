#pragma once

#include <Engine.Graphics/Texture/TextureLikeObject.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Resource;
#endif

namespace hg::engine::graphics {
    typedef resource::Resource<mpt<TextureLikeObject>> TextureResource;
}

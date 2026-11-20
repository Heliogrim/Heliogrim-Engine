#pragma once

#include <Engine.Graphics/Texture/TextureLikeObject.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    typedef resource::Resource<mpt<TextureLikeObject>> TextureResource;
}

#pragma once

#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX/Texture/TextureLikeObject.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

namespace hg::engine::gfx {
    typedef resource::Resource<mpt<TextureLikeObject>> TextureResource;
}

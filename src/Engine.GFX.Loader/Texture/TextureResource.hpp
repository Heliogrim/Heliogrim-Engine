#pragma once

#include <Engine.GFX/Texture/ProxyTexture.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

namespace hg::engine::gfx {
    typedef resource::Resource<ProxyTexture<ptr>> TextureResource;
}

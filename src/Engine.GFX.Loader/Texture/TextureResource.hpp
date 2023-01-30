#pragma once

#include <Engine.GFX/Texture/ProxyTexture.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

namespace ember::engine::gfx {
    typedef resource::Resource<ProxyTexture<non_owning_rptr>> TextureResource;
}

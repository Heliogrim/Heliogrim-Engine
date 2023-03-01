#include "Icon3d.hpp"
#include "../Images/Icon3d.hpp"

#include <Heliogrim/TextureFormat.hpp>

using namespace hg::game::assets;
using namespace hg;

texture::Icon3d::Icon3d() :
    TextureAsset(
        texture::Icon3d::unstable_auto_guid(),
        image::Icon3d::unstable_auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
